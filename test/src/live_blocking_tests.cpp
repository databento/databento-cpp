#include <gtest/gtest.h>
#include <openssl/sha.h>  //  SHA256_DIGEST_LENGTH

#include <atomic>
#include <chrono>  // milliseconds
#include <condition_variable>
#include <memory>
#include <mutex>   // lock_guard, mutex, unique_lock
#include <thread>  // this_thread
#include <vector>

#include "databento/constants.hpp"  // dataset
#include "databento/datetime.hpp"
#include "databento/enums.hpp"  // Schema, SType
#include "databento/exceptions.hpp"
#include "databento/live_blocking.hpp"
#include "databento/log.hpp"
#include "databento/record.hpp"
#include "databento/symbology.hpp"
#include "databento/with_ts_out.hpp"
#include "mock/mock_lsg_server.hpp"  // MockLsgServer

namespace databento {
namespace test {
class LiveBlockingTests : public testing::Test {
 protected:
  template <typename T>
  static constexpr RecordHeader DummyHeader(RType rtype) {
    return {sizeof(T) / RecordHeader::kLengthMultiplier, rtype, 1, 1,
            UnixNanos{}};
  }

  static constexpr auto kKey = "32-character-with-lots-of-filler";
  static constexpr auto kLocalhost = "127.0.0.1";

  std::unique_ptr<ILogReceiver> logger_{new NullLogReceiver};
};

TEST_F(LiveBlockingTests, TestAuthentication) {
  constexpr auto kTsOut = false;
  const mock::MockLsgServer mock_server{dataset::kXnasItch, kTsOut,
                                        [](mock::MockLsgServer& self) {
                                          self.Accept();
                                          self.Authenticate();
                                        }};

  const LiveBlocking target{
      logger_.get(),      kKey,   dataset::kXnasItch,    kLocalhost,
      mock_server.Port(), kTsOut, VersionUpgradePolicy{}};
}

TEST_F(LiveBlockingTests, TestStart) {
  constexpr auto kTsOut = false;
  const mock::MockLsgServer mock_server{dataset::kGlbxMdp3, kTsOut,
                                        [](mock::MockLsgServer& self) {
                                          self.Accept();
                                          self.Authenticate();
                                          self.Start();
                                        }};

  LiveBlocking target{
      logger_.get(),      kKey,   dataset::kGlbxMdp3,    kLocalhost,
      mock_server.Port(), kTsOut, VersionUpgradePolicy{}};
  const auto metadata = target.Start();
  EXPECT_EQ(metadata.version, 1);
  EXPECT_TRUE(metadata.has_mixed_schema);
  EXPECT_EQ(metadata.dataset, dataset::kGlbxMdp3);
}

TEST_F(LiveBlockingTests, TestSubscribe) {
  constexpr auto kTsOut = false;
  constexpr auto kDataset = dataset::kXnasItch;
  const std::vector<std::string> kSymbols{"MSFT", "TSLA", "QQQ"};
  const auto kSchema = Schema::Ohlcv1M;
  const auto kSType = SType::RawSymbol;

  const mock::MockLsgServer mock_server{
      kDataset, kTsOut,
      [&kSymbols, kSchema, kSType](mock::MockLsgServer& self) {
        self.Accept();
        self.Authenticate();
        self.Subscribe(kSymbols, kSchema, kSType);
      }};

  LiveBlocking target{logger_.get(),
                      kKey,
                      kDataset,
                      kLocalhost,
                      mock_server.Port(),
                      kTsOut,
                      VersionUpgradePolicy{}};
  target.Subscribe(kSymbols, kSchema, kSType);
}

TEST_F(LiveBlockingTests, TestSubscriptionChunking) {
  constexpr auto kTsOut = false;
  constexpr auto kDataset = dataset::kXnasItch;
  const auto kSymbol = "TEST";
  const std::size_t kSymbolCount = 1000;
  const auto kSchema = Schema::Ohlcv1M;
  const auto kSType = SType::RawSymbol;

  const mock::MockLsgServer mock_server{
      kDataset, kTsOut,
      [kSymbol, kSymbolCount, kSchema, kSType](mock::MockLsgServer& self) {
        self.Accept();
        self.Authenticate();
        std::size_t i{};
        while (i < 1000) {
          const auto chunk_size =
              std::min(static_cast<std::size_t>(128), kSymbolCount - i);
          const std::vector<std::string> symbols_chunk(chunk_size, kSymbol);
          self.Subscribe(symbols_chunk, kSchema, kSType);
          i += chunk_size;
        }
      }};

  LiveBlocking target{logger_.get(),
                      kKey,
                      kDataset,
                      kLocalhost,
                      mock_server.Port(),
                      kTsOut,
                      VersionUpgradePolicy{}};
  const std::vector<std::string> kSymbols(kSymbolCount, kSymbol);
  target.Subscribe(kSymbols, kSchema, kSType);
}

TEST_F(LiveBlockingTests, TestNextRecord) {
  constexpr auto kTsOut = false;
  const auto kRecCount = 12;
  constexpr OhlcvMsg kRec{DummyHeader<OhlcvMsg>(RType::Ohlcv1M), 1, 2, 3, 4, 5};
  const mock::MockLsgServer mock_server{
      dataset::kXnasItch, kTsOut, [kRec, kRecCount](mock::MockLsgServer& self) {
        self.Accept();
        self.Authenticate();
        for (size_t i = 0; i < kRecCount; ++i) {
          self.SendRecord(kRec);
        }
      }};

  LiveBlocking target{
      logger_.get(),      kKey,   dataset::kXnasItch,    kLocalhost,
      mock_server.Port(), kTsOut, VersionUpgradePolicy{}};
  for (size_t i = 0; i < kRecCount; ++i) {
    const auto rec = target.NextRecord();
    ASSERT_TRUE(rec.Holds<OhlcvMsg>()) << "Failed on call " << i;
    EXPECT_EQ(rec.Get<OhlcvMsg>(), kRec);
  }
}

TEST_F(LiveBlockingTests, TestNextRecordTimeout) {
  constexpr std::chrono::milliseconds kTimeout{50};
  constexpr auto kTsOut = false;
  constexpr Mbp1Msg kRec{DummyHeader<Mbp1Msg>(RType::Mbp1),
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
      dataset::kXnasItch, kTsOut, [&](mock::MockLsgServer& self) {
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

  LiveBlocking target{
      logger_.get(),      kKey,   dataset::kXnasItch,    kLocalhost,
      mock_server.Port(), kTsOut, VersionUpgradePolicy{}};
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
  constexpr auto kTsOut = false;
  constexpr MboMsg kRec{DummyHeader<MboMsg>(RType::Mbo),
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

  bool send_remaining{};
  std::mutex send_remaining_mutex;
  std::condition_variable send_remaining_cv;
  const mock::MockLsgServer mock_server{
      dataset::kGlbxMdp3, kTsOut,
      [kRec, &send_remaining, &send_remaining_mutex,
       &send_remaining_cv](mock::MockLsgServer& self) {
        self.Accept();
        self.Authenticate();
        self.SendRecord(kRec);
        // should cause partial read
        self.SplitSendRecord(kRec, send_remaining, send_remaining_mutex,
                             send_remaining_cv);
      }};

  LiveBlocking target{
      logger_.get(),      kKey,   dataset::kGlbxMdp3,    kLocalhost,
      mock_server.Port(), kTsOut, VersionUpgradePolicy{}};
  auto rec = target.NextRecord();
  ASSERT_TRUE(rec.Holds<MboMsg>());
  EXPECT_EQ(rec.Get<MboMsg>(), kRec);
  // partial read and timeout occurs here
  ASSERT_EQ(target.NextRecord(std::chrono::milliseconds{10}), nullptr);
  {
    const std::lock_guard<std::mutex> lock{send_remaining_mutex};
    send_remaining = true;
    // notify server to send remaining part of record
    send_remaining_cv.notify_one();
  }
  // recovers from partial read
  rec = target.NextRecord();
  ASSERT_TRUE(rec.Holds<MboMsg>());
  EXPECT_EQ(rec.Get<MboMsg>(), kRec);
}

TEST_F(LiveBlockingTests, TestNextRecordWithTsOut) {
  const auto kRecCount = 5;
  constexpr auto kTsOut = true;
  const WithTsOut<TradeMsg> send_rec{
      {DummyHeader<TradeMsg>(RType::Mbp0),
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
      dataset::kXnasItch, kTsOut,
      [send_rec, kRecCount](mock::MockLsgServer& self) {
        self.Accept();
        self.Authenticate();
        for (size_t i = 0; i < kRecCount; ++i) {
          self.SendRecord(send_rec);
        }
      }};

  LiveBlocking target{
      logger_.get(),      kKey,   dataset::kXnasItch,    kLocalhost,
      mock_server.Port(), kTsOut, VersionUpgradePolicy{}};
  for (size_t i = 0; i < kRecCount; ++i) {
    const auto rec = target.NextRecord();
    ASSERT_TRUE(rec.Holds<WithTsOut<TradeMsg>>()) << "Failed on call " << i;
    EXPECT_EQ(rec.Get<WithTsOut<TradeMsg>>(), send_rec);
    // Extracting the plain record (without ts_out) should also work
    ASSERT_TRUE(rec.Holds<TradeMsg>()) << "Failed on call " << i;
    EXPECT_EQ(rec.Get<TradeMsg>(), send_rec.rec);
  }
}

TEST_F(LiveBlockingTests, TestStop) {
  constexpr auto kTsOut = true;
  const WithTsOut<TradeMsg> send_rec{
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
  std::unique_ptr<mock::MockLsgServer> mock_server{
      new mock::MockLsgServer{
          dataset::kXnasItch, kTsOut,
          [send_rec, &has_stopped](mock::MockLsgServer& self) {
            self.Accept();
            self.Authenticate();
            self.SendRecord(send_rec);
            while (!has_stopped) {
              std::this_thread::yield();
            }
            const std::string rec_str{reinterpret_cast<const char*>(&send_rec),
                                      sizeof(send_rec)};
            while (self.UncheckedSend(rec_str) ==
                   static_cast<::ssize_t>(rec_str.size())) {
            }
          }}  // namespace test
  };  // namespace databento

  LiveBlocking target{
      logger_.get(),       kKey,   dataset::kXnasItch,    kLocalhost,
      mock_server->Port(), kTsOut, VersionUpgradePolicy{}};
  ASSERT_EQ(target.NextRecord().Get<WithTsOut<TradeMsg>>(), send_rec);
  target.Stop();
  has_stopped = true;
  // kill mock server and join thread before client goes out of scope
  // to ensure Stop is killing the connection, not the client's destructor
  mock_server.reset();
}

TEST_F(LiveBlockingTests, TestConnectWhenGatewayNotUp) {
  constexpr auto kTsOut = true;
  ASSERT_THROW(LiveBlocking(logger_.get(), kKey, dataset::kXnasItch, kLocalhost,
                            80, kTsOut, VersionUpgradePolicy{}),
               databento::TcpError);
}

TEST_F(LiveBlockingTests, TestReconnect) {
  constexpr auto kTsOut = false;
  constexpr TradeMsg kRec{DummyHeader<TradeMsg>(RType::Mbp0),
                          1,
                          2,
                          Action::Add,
                          Side::Ask,
                          {},
                          1,
                          {},
                          {},
                          2};

  bool should_close{};
  std::mutex should_close_mutex;
  std::condition_variable should_close_cv;
  bool has_closed{};
  std::mutex has_closed_mutex;
  std::condition_variable has_closed_cv;
  std::unique_ptr<mock::MockLsgServer> mock_server{new mock::MockLsgServer{
      dataset::kXnasItch, kTsOut,
      [kRec, &has_closed, &has_closed_cv, &has_closed_mutex, &should_close,
       &should_close_cv, &should_close_mutex](mock::MockLsgServer& self) {
        self.Accept();
        self.Authenticate();
        {
          std::unique_lock<std::mutex> lock{should_close_mutex};
          should_close_cv.wait(lock, [&should_close] { return should_close; });
        }
        // Close connection
        self.Close();
        {
          const std::lock_guard<std::mutex> _lock{has_closed_mutex};
          has_closed = true;
          has_closed_cv.notify_one();
        }
        // Wait for reconnect
        self.Accept();
        self.Authenticate();
        self.Subscribe(kAllSymbols, Schema::Trades, SType::RawSymbol);
        self.Start();
        self.SendRecord(kRec);
      }}};
  LiveBlocking target{
      logger_.get(),       kKey,   dataset::kXnasItch,    kLocalhost,
      mock_server->Port(), kTsOut, VersionUpgradePolicy{}};
  // Tell server to close connection
  {
    const std::lock_guard<std::mutex> _lock{should_close_mutex};
    should_close = true;
    should_close_cv.notify_one();
  }
  // Wait for server to close connection
  {
    std::unique_lock<std::mutex> lock{has_closed_mutex};
    has_closed_cv.wait(lock, [&has_closed] { return has_closed; });
  }
  ASSERT_THROW(target.NextRecord(), databento::DbnResponseError);
  target.Reconnect();
  target.Subscribe(kAllSymbols, Schema::Trades, SType::RawSymbol);
  const auto metadata = target.Start();
  EXPECT_TRUE(metadata.has_mixed_schema);
  const auto rec = target.NextRecord();
  ASSERT_TRUE(rec.Holds<TradeMsg>());
  ASSERT_EQ(rec.Get<TradeMsg>(), kRec);
}
}  // namespace test
}  // namespace databento
