#include <gtest/gtest.h>
#include <openssl/sha.h>  //  SHA256_DIGEST_LENGTH

#include <atomic>
#include <chrono>  // milliseconds
#include <condition_variable>
#include <mutex>   // lock_guard, mutex, unique_lock
#include <thread>  // this_thread
#include <vector>

#include "databento/constants.hpp"  // dataset
#include "databento/datetime.hpp"
#include "databento/enums.hpp"  // Schema, SType
#include "databento/live_blocking.hpp"
#include "databento/record.hpp"
#include "databento/with_ts_out.hpp"
#include "mock/mock_lsg_server.hpp"  // MockLsgServer

namespace databento {
namespace test {
class LiveBlockingTests : public testing::Test {
 protected:
  static constexpr auto kKey = "32-character-with-lots-of-filler";

  template <typename T>
  static RecordHeader DummyHeader(RType rtype) {
    return {sizeof(T) / 4, rtype, 1, 1, UnixNanos{}};
  }
};

TEST_F(LiveBlockingTests, TestAuthentication) {
  const mock::MockLsgServer mock_server{dataset::kXnasItch,
                                        [](mock::MockLsgServer& self) {
                                          self.Accept();
                                          self.Authenticate();
                                        }};

  const LiveBlocking target{kKey, dataset::kXnasItch, "127.0.0.1",
                            mock_server.Port(), false};
}

TEST_F(LiveBlockingTests, TestStart) {
  constexpr auto kSchema = Schema::Mbo;
  const mock::MockLsgServer mock_server{dataset::kGlbxMdp3,
                                        [](mock::MockLsgServer& self) {
                                          self.Accept();
                                          self.Authenticate();
                                          self.Start(kSchema);
                                        }};

  LiveBlocking target{kKey, dataset::kGlbxMdp3, "127.0.0.1", mock_server.Port(),
                      false};
  const auto metadata = target.Start();
  EXPECT_EQ(metadata.version, 1);
  EXPECT_EQ(metadata.schema, kSchema);
  EXPECT_EQ(metadata.dataset, dataset::kGlbxMdp3);
}

TEST_F(LiveBlockingTests, TestSubscribe) {
  constexpr auto kDataset = dataset::kXnasItch;
  const std::vector<std::string> kSymbols{"MSFT", "TSLA", "QQQ"};
  constexpr auto kSchema = Schema::Ohlcv1M;
  constexpr auto kSType = SType::RawSymbol;

  const mock::MockLsgServer mock_server{
      kDataset, [&kSymbols](mock::MockLsgServer& self) {
        self.Accept();
        self.Authenticate();
        self.Subscribe(kSymbols, kSchema, kSType);
      }};

  LiveBlocking target{kKey, kDataset, "127.0.0.1", mock_server.Port(), false};
  target.Subscribe(kSymbols, kSchema, kSType);
}

TEST_F(LiveBlockingTests, TestNextRecord) {
  constexpr auto kRecCount = 12;
  const OhlcvMsg kRec{DummyHeader<OhlcvMsg>(RType::Ohlcv1M), 1, 2, 3, 4, 5};
  const mock::MockLsgServer mock_server{
      dataset::kXnasItch, [kRec](mock::MockLsgServer& self) {
        self.Accept();
        self.Authenticate();
        for (size_t i = 0; i < kRecCount; ++i) {
          self.SendRecord(kRec);
        }
      }};

  LiveBlocking target{kKey, dataset::kXnasItch, "127.0.0.1", mock_server.Port(),
                      false};
  for (size_t i = 0; i < kRecCount; ++i) {
    const auto rec = target.NextRecord();
    ASSERT_TRUE(rec.Holds<OhlcvMsg>()) << "Failed on call " << i;
    EXPECT_EQ(rec.Get<OhlcvMsg>(), kRec);
  }
}

TEST_F(LiveBlockingTests, TestNextRecordTimeout) {
  constexpr std::chrono::milliseconds kTimeout{50};
  const Mbp1Msg kRec{DummyHeader<Mbp1Msg>(RType::Mbp1),
                     1,
                     2,
                     Action::Add,
                     Side::Bid,
                     {},
                     1,
                     UnixNanos{},
                     TimeDeltaNanos{},
                     10,
                     {BidAskPair{1, 2, 3, 4, 5, 6}}};

  bool sent_first_msg = false;
  std::mutex send_mutex;
  std::condition_variable send_cv;
  bool received_first_msg = false;
  std::mutex receive_mutex;
  std::condition_variable receive_cv;
  const mock::MockLsgServer mock_server{
      dataset::kXnasItch, [&](mock::MockLsgServer& self) {
        self.Accept();
        self.Authenticate();
        self.SendRecord(kRec);
        {
          // notify client the first record's been sent
          const std::lock_guard<std::mutex> lock{send_mutex};
          sent_first_msg = true;
          send_cv.notify_one();
        }
        {
          // wait for client to read first record
          std::unique_lock<std::mutex> lock{receive_mutex};
          receive_cv.wait(lock,
                          [&received_first_msg] { return received_first_msg; });
        }
        self.SendRecord(kRec);
      }};

  LiveBlocking target{kKey, dataset::kXnasItch, "127.0.0.1", mock_server.Port(),
                      false};
  {
    // wait for server to send first record to avoid flaky timeouts
    std::unique_lock<std::mutex> lock{send_mutex};
    send_cv.wait(lock, [&sent_first_msg] { return sent_first_msg; });
  }
  auto* rec = target.NextRecord(kTimeout);
  ASSERT_NE(rec, nullptr);
  EXPECT_TRUE(rec->Holds<Mbp1Msg>());
  EXPECT_EQ(rec->Get<Mbp1Msg>(), kRec);
  rec = target.NextRecord(kTimeout);
  EXPECT_EQ(rec, nullptr) << "Did not timeout when expected";
  {
    // notify server the timeout occurred
    const std::lock_guard<std::mutex> lock{receive_mutex};
    received_first_msg = true;
    receive_cv.notify_one();
  }
  rec = target.NextRecord(kTimeout);
  ASSERT_NE(rec, nullptr);
  EXPECT_TRUE(rec->Holds<Mbp1Msg>());
  EXPECT_EQ(rec->Get<Mbp1Msg>(), kRec);
}

TEST_F(LiveBlockingTests, TestNextRecordPartialRead) {
  const MboMsg kRec{DummyHeader<MboMsg>(RType::Mbo),
                    1,
                    2,
                    3,
                    {},
                    4,
                    Action::Add,
                    Side::Bid,
                    UnixNanos{},
                    TimeDeltaNanos{},
                    100};

  std::mutex mutex;
  std::condition_variable cv;
  const mock::MockLsgServer mock_server{
      dataset::kGlbxMdp3, [kRec, &mutex, &cv](mock::MockLsgServer& self) {
        self.Accept();
        self.Authenticate();
        self.SendRecord(kRec);
        // should cause partial read
        self.SplitSendRecord(kRec, mutex, cv);
      }};

  LiveBlocking target{kKey, dataset::kGlbxMdp3, "127.0.0.1", mock_server.Port(),
                      false};
  auto rec = target.NextRecord();
  ASSERT_TRUE(rec.Holds<MboMsg>());
  EXPECT_EQ(rec.Get<MboMsg>(), kRec);
  // partial read and timeout occurs here
  ASSERT_EQ(target.NextRecord(std::chrono::milliseconds{10}), nullptr);
  {
    const std::lock_guard<std::mutex> lock{mutex};
    // notify server to send remaining part of record
    cv.notify_one();
  }
  // recovers from partial read
  rec = target.NextRecord();
  ASSERT_TRUE(rec.Holds<MboMsg>());
  EXPECT_EQ(rec.Get<MboMsg>(), kRec);
}

TEST_F(LiveBlockingTests, TestNextRecordWithTsOut) {
  constexpr auto kRecCount = 5;
  const WithTsOut<TradeMsg> kRec{
      {DummyHeader<WithTsOut<TradeMsg>>(RType::Mbp0),
       1,
       2,
       Action::Add,
       Side::Ask,
       {},
       1,
       {},
       {},
       2},
      UnixNanos{std::chrono::seconds{1678910279000000000}}};
  const mock::MockLsgServer mock_server{
      dataset::kXnasItch, [kRec](mock::MockLsgServer& self) {
        self.Accept();
        self.Authenticate();
        for (size_t i = 0; i < kRecCount; ++i) {
          self.SendRecord(kRec);
        }
      }};

  LiveBlocking target{kKey, dataset::kXnasItch, "127.0.0.1", mock_server.Port(),
                      false};
  for (size_t i = 0; i < kRecCount; ++i) {
    const auto rec = target.NextRecord();
    ASSERT_TRUE(rec.Holds<WithTsOut<TradeMsg>>()) << "Failed on call " << i;
    EXPECT_EQ(rec.Get<WithTsOut<TradeMsg>>(), kRec);
    // Extracting the plain record (without ts_out) should also work
    ASSERT_TRUE(rec.Holds<TradeMsg>()) << "Failed on call " << i;
    EXPECT_EQ(rec.Get<TradeMsg>(), kRec.rec);
  }
}

TEST_F(LiveBlockingTests, TestStop) {
  const WithTsOut<TradeMsg> kRec{
      {DummyHeader<WithTsOut<TradeMsg>>(RType::Mbp0),
       1,
       2,
       Action::Add,
       Side::Ask,
       {},
       1,
       {},
       {},
       2},
      UnixNanos{std::chrono::seconds{1678910279000000000}}};
  std::atomic<bool> has_stopped{false};
  const mock::MockLsgServer mock_server{
      dataset::kXnasItch, [kRec, &has_stopped](mock::MockLsgServer& self) {
        self.Accept();
        self.Authenticate();
        self.SendRecord(kRec);
        while (!has_stopped) {
          std::this_thread::yield();
        }
        const std::string rec_str{reinterpret_cast<const char*>(&kRec),
                                  sizeof(kRec)};
        for (size_t i = 0; i < 5; ++i) {
          if (self.UncheckedSend(rec_str) <
              static_cast<::ssize_t>(rec_str.size())) {
            return;
          }
        }
        FAIL() << "Connection remained open";
      }};

  LiveBlocking target{kKey, dataset::kXnasItch, "127.0.0.1", mock_server.Port(),
                      false};
  ASSERT_EQ(target.NextRecord().Get<WithTsOut<TradeMsg>>(), kRec);
  target.Stop();
  has_stopped = true;
  std::this_thread::sleep_for(std::chrono::milliseconds{50});
}
}  // namespace test
}  // namespace databento
