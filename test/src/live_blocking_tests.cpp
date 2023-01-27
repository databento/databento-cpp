#include <gtest/gtest.h>
#include <openssl/sha.h>  //  SHA256_DIGEST_LENGTH

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
  const mock::MockLsgServer mock_server{[](mock::MockLsgServer& self) {
    self.Accept();
    self.Authenticate();
  }};

  const LiveBlocking target{kKey, "127.0.0.1", mock_server.Port(), false};
}

TEST_F(LiveBlockingTests, TestStart) {
  const mock::MockLsgServer mock_server{[](mock::MockLsgServer& self) {
    self.Accept();
    self.Authenticate();
    self.Start();
  }};

  LiveBlocking target{kKey, "127.0.0.1", mock_server.Port(), false};
  target.Start();
}

TEST_F(LiveBlockingTests, TestSubscribe) {
  constexpr auto kDataset = dataset::kXnasItch;
  const std::vector<std::string> kSymbols{"MSFT", "TSLA", "QQQ"};
  constexpr auto kSchema = Schema::Ohlcv1M;
  constexpr auto kSType = SType::Native;

  const mock::MockLsgServer mock_server{[&kSymbols](mock::MockLsgServer& self) {
    self.Accept();
    self.Authenticate();
    self.Subscribe(kDataset, kSymbols, kSchema, kSType);
  }};

  LiveBlocking target{kKey, "127.0.0.1", mock_server.Port(), false};
  target.Subscribe(kDataset, kSymbols, kSchema, kSType);
}

TEST_F(LiveBlockingTests, TestNextRecord) {
  constexpr auto kRecCount = 12;
  const OhlcvMsg kRec{DummyHeader<OhlcvMsg>(RType::Ohlcv), 1, 2, 3, 4, 5};
  const mock::MockLsgServer mock_server{[kRec](mock::MockLsgServer& self) {
    self.Accept();
    self.Authenticate();
    for (size_t i = 0; i < kRecCount; ++i) {
      self.SendRecord(kRec);
    }
  }};

  LiveBlocking target{kKey, "127.0.0.1", mock_server.Port(), false};
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
                     'A',
                     'B',
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
  const mock::MockLsgServer mock_server{[&](mock::MockLsgServer& self) {
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

  LiveBlocking target{kKey, "127.0.0.1", mock_server.Port(), false};
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
                    'A',
                    'B',
                    UnixNanos{},
                    TimeDeltaNanos{},
                    100};

  std::mutex mutex;
  std::condition_variable cv;
  const mock::MockLsgServer mock_server{
      [kRec, &mutex, &cv](mock::MockLsgServer& self) {
        self.Accept();
        self.Authenticate();
        self.SendRecord(kRec);
        // should cause partial read
        self.SplitSendRecord(kRec, mutex, cv);
      }};

  LiveBlocking target{kKey, "127.0.0.1", mock_server.Port(), false};
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
}  // namespace test
}  // namespace databento
