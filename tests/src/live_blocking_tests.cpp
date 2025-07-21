#include <gtest/gtest.h>
#include <openssl/sha.h>  //  SHA256_DIGEST_LENGTH

#include <atomic>
#include <chrono>  // milliseconds
#include <condition_variable>
#include <memory>
#include <mutex>   // lock_guard, mutex, unique_lock
#include <thread>  // this_thread
#include <utility>
#include <variant>
#include <vector>

#include "databento/constants.hpp"  // dataset
#include "databento/datetime.hpp"
#include "databento/enums.hpp"  // Schema, SType
#include "databento/exceptions.hpp"
#include "databento/live.hpp"
#include "databento/live_blocking.hpp"
#include "databento/live_subscription.hpp"
#include "databento/log.hpp"
#include "databento/record.hpp"
#include "databento/symbology.hpp"
#include "databento/with_ts_out.hpp"
#include "mock/mock_log_receiver.hpp"
#include "mock/mock_lsg_server.hpp"  // MockLsgServer

namespace databento::tests {
class LiveBlockingTests : public testing::Test {
 protected:
  template <typename T>
  static constexpr RecordHeader DummyHeader(RType rtype) {
    return {sizeof(T) / RecordHeader::kLengthMultiplier, rtype, 1, 1, UnixNanos{}};
  }

  static constexpr auto kKey = "32-character-with-lots-of-filler";
  static constexpr auto kLocalhost = "127.0.0.1";

  mock::MockLogReceiver logger_ =
      mock::MockLogReceiver::AssertNoLogs(LogLevel::Warning);
  LiveBuilder builder_{LiveBuilder{}.SetLogReceiver(&logger_).SetKey(kKey)};
};

TEST_F(LiveBlockingTests, TestAuthentication) {
  constexpr auto kTsOut = false;
  constexpr auto kHeartbeatInterval = std::chrono::seconds{10};
  const mock::MockLsgServer mock_server{dataset::kXnasItch, kTsOut, kHeartbeatInterval,
                                        [](mock::MockLsgServer& self) {
                                          self.Accept();
                                          self.Authenticate();
                                        }};

  const LiveBlocking target = builder_.SetDataset(dataset::kXnasItch)
                                  .SetHeartbeatInterval(kHeartbeatInterval)
                                  .SetAddress(kLocalhost, mock_server.Port())
                                  .BuildBlocking();
}

TEST_F(LiveBlockingTests, TestStartAndUpgrade) {
  constexpr auto kTsOut = true;
  for (const auto [upgrade_policy, exp_version] :
       {std::make_pair(VersionUpgradePolicy::AsIs, 1),
        std::make_pair(VersionUpgradePolicy::UpgradeToV2, 2),
        std::make_pair(VersionUpgradePolicy::UpgradeToV3, 3)}) {
    const mock::MockLsgServer mock_server{dataset::kGlbxMdp3, kTsOut,
                                          [](mock::MockLsgServer& self) {
                                            self.Accept();
                                            self.Authenticate();
                                            self.Start();
                                          }};

    LiveBlocking target = builder_.SetAddress(kLocalhost, mock_server.Port())
                              .SetSendTsOut(kTsOut)
                              .SetDataset(dataset::kGlbxMdp3)
                              .SetUpgradePolicy(upgrade_policy)
                              .BuildBlocking();
    const auto metadata = target.Start();
    EXPECT_EQ(metadata.version, exp_version);
    EXPECT_FALSE(metadata.schema.has_value());
    EXPECT_EQ(metadata.dataset, dataset::kGlbxMdp3);
  }
}

TEST_F(LiveBlockingTests, TestSubscribe) {
  constexpr auto kTsOut = false;
  constexpr auto kDataset = dataset::kXnasItch;
  const std::vector<std::string> kSymbols{"MSFT", "TSLA", "QQQ"};
  const auto kSchema = Schema::Ohlcv1M;
  const auto kSType = SType::RawSymbol;

  const mock::MockLsgServer mock_server{
      kDataset, kTsOut, [&kSymbols, kSchema, kSType](mock::MockLsgServer& self) {
        self.Accept();
        self.Authenticate();
        self.Subscribe(kSymbols, kSchema, kSType, true);
      }};

  LiveBlocking target = builder_.SetDataset(kDataset)
                            .SetSendTsOut(kTsOut)
                            .SetAddress(kLocalhost, mock_server.Port())
                            .BuildBlocking();
  target.Subscribe(kSymbols, kSchema, kSType);
}

TEST_F(LiveBlockingTests, TestSubscriptionChunkingUnixNanos) {
  constexpr auto kTsOut = false;
  constexpr auto kDataset = dataset::kXnasItch;
  const auto kSymbol = "TEST";
  const std::size_t kSymbolCount = 1001;
  const auto kSchema = Schema::Ohlcv1M;
  const auto kSType = SType::RawSymbol;

  const mock::MockLsgServer mock_server{
      kDataset, kTsOut,
      [kSymbol, kSymbolCount, kSchema, kSType](mock::MockLsgServer& self) {
        self.Accept();
        self.Authenticate();
        std::size_t i{};
        while (i < kSymbolCount) {
          const auto chunk_size =
              std::min(static_cast<std::size_t>(500), kSymbolCount - i);
          const std::vector<std::string> symbols_chunk(chunk_size, kSymbol);
          self.Subscribe(symbols_chunk, kSchema, kSType,
                         i + chunk_size == kSymbolCount);
          i += chunk_size;
        }
      }};

  LiveBlocking target = builder_.SetDataset(kDataset)
                            .SetSendTsOut(kTsOut)
                            .SetAddress(kLocalhost, mock_server.Port())
                            .BuildBlocking();
  const std::vector<std::string> kSymbols(kSymbolCount, kSymbol);
  target.Subscribe(kSymbols, kSchema, kSType);
}

TEST_F(LiveBlockingTests, TestSubscriptionUnixNanos0) {
  constexpr auto kTsOut = false;
  constexpr auto kDataset = dataset::kXnasItch;
  const std::vector<std::string> kSymbols = {"TEST1", "TEST2"};
  const auto kSchema = Schema::Ohlcv1M;
  const auto kSType = SType::RawSymbol;
  const auto kStart = UnixNanos{};

  const mock::MockLsgServer mock_server{
      kDataset, kTsOut,
      [&kSymbols, kSchema, kSType, kStart](mock::MockLsgServer& self) {
        self.Accept();
        self.Authenticate();
        std::size_t i{};
        self.Subscribe(kSymbols, kSchema, kSType, "0", true);
      }};

  LiveBlocking target = builder_.SetDataset(kDataset)
                            .SetSendTsOut(kTsOut)
                            .SetAddress(kLocalhost, mock_server.Port())
                            .BuildBlocking();
  target.Subscribe(kSymbols, kSchema, kSType, kStart);
}

TEST_F(LiveBlockingTests, TestSubscriptionChunkingStringStart) {
  constexpr auto kTsOut = false;
  constexpr auto kDataset = dataset::kXnasItch;
  const auto kSymbol = "TEST";
  const std::size_t kSymbolCount = 1001;
  const auto kSchema = Schema::Ohlcv1M;
  const auto kSType = SType::RawSymbol;
  const auto kStart = "2020-01-01T00:00:00";

  const mock::MockLsgServer mock_server{
      kDataset, kTsOut,
      [kSymbol, kSymbolCount, kSchema, kSType, kStart](mock::MockLsgServer& self) {
        self.Accept();
        self.Authenticate();
        std::size_t i{};
        while (i < kSymbolCount) {
          const auto chunk_size =
              std::min(static_cast<std::size_t>(500), kSymbolCount - i);
          const std::vector<std::string> symbols_chunk(chunk_size, kSymbol);
          self.Subscribe(symbols_chunk, kSchema, kSType, kStart,
                         i + chunk_size == kSymbolCount);
          i += chunk_size;
        }
      }};

  LiveBlocking target = builder_.SetDataset(kDataset)
                            .SetSendTsOut(kTsOut)
                            .SetAddress(kLocalhost, mock_server.Port())
                            .BuildBlocking();
  const std::vector<std::string> kSymbols(kSymbolCount, kSymbol);
  target.Subscribe(kSymbols, kSchema, kSType, kStart);
}

TEST_F(LiveBlockingTests, TestSubscribeSnapshot) {
  constexpr auto kTsOut = false;
  constexpr auto kDataset = dataset::kXnasItch;
  const auto kSymbol = "TEST";
  const std::size_t kSymbolCount = 1001;
  const auto kSchema = Schema::Ohlcv1M;
  const auto kSType = SType::RawSymbol;
  const auto kUseSnapshot = true;

  const mock::MockLsgServer mock_server{
      kDataset, kTsOut,
      [kSymbol, kSymbolCount, kSchema, kSType,
       kUseSnapshot](mock::MockLsgServer& self) {
        self.Accept();
        self.Authenticate();
        std::size_t i{};
        while (i < kSymbolCount) {
          const auto chunk_size =
              std::min(static_cast<std::size_t>(500), kSymbolCount - i);
          const std::vector<std::string> symbols_chunk(chunk_size, kSymbol);
          self.SubscribeWithSnapshot(symbols_chunk, kSchema, kSType,
                                     i + chunk_size == kSymbolCount);
          i += chunk_size;
        }
      }};

  LiveBlocking target = builder_.SetDataset(kDataset)
                            .SetSendTsOut(kTsOut)
                            .SetAddress(kLocalhost, mock_server.Port())
                            .BuildBlocking();
  const std::vector<std::string> kSymbols(kSymbolCount, kSymbol);
  target.SubscribeWithSnapshot(kSymbols, kSchema, kSType);
}

TEST_F(LiveBlockingTests, TestInvalidSubscription) {
  constexpr auto kTsOut = false;
  constexpr auto kDataset = dataset::kXnasItch;
  const std::vector<std::string> kNoSymbols{};
  const auto kSchema = Schema::Ohlcv1M;
  const auto kSType = SType::RawSymbol;

  const mock::MockLsgServer mock_server{kDataset, kTsOut,
                                        [](mock::MockLsgServer& self) {
                                          self.Accept();
                                          self.Authenticate();
                                        }};

  LiveBlocking target = builder_.SetDataset(kDataset)
                            .SetSendTsOut(kTsOut)
                            .SetAddress(kLocalhost, mock_server.Port())
                            .BuildBlocking();

  ASSERT_THROW(target.Subscribe(kNoSymbols, kSchema, kSType),
               databento::InvalidArgumentError);
}

TEST_F(LiveBlockingTests, TestNextRecord) {
  constexpr auto kTsOut = false;
  const auto kRecCount = 12;
  constexpr OhlcvMsg kRec{DummyHeader<OhlcvMsg>(RType::Ohlcv1M), 1, 2, 3, 4, 5};
  const mock::MockLsgServer mock_server{dataset::kXnasItch, kTsOut,
                                        [kRec, kRecCount](mock::MockLsgServer& self) {
                                          self.Accept();
                                          self.Authenticate();
                                          for (size_t i = 0; i < kRecCount; ++i) {
                                            self.SendRecord(kRec);
                                          }
                                        }};

  LiveBlocking target = builder_.SetDataset(dataset::kXnasItch)
                            .SetSendTsOut(kTsOut)
                            .SetAddress(kLocalhost, mock_server.Port())
                            .BuildBlocking();
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
          receive_cv.wait(lock, [&received_first_msg] { return received_first_msg; });
        }
        self.SendRecord(kRec);
      }};

  LiveBlocking target = builder_.SetDataset(dataset::kXnasItch)
                            .SetSendTsOut(kTsOut)
                            .SetAddress(kLocalhost, mock_server.Port())
                            .BuildBlocking();
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

  LiveBlocking target = builder_.SetDataset(dataset::kGlbxMdp3)
                            .SetSendTsOut(kTsOut)
                            .SetAddress(kLocalhost, mock_server.Port())
                            .BuildBlocking();
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
      dataset::kXnasItch, kTsOut, [send_rec, kRecCount](mock::MockLsgServer& self) {
        self.Accept();
        self.Authenticate();
        for (size_t i = 0; i < kRecCount; ++i) {
          self.SendRecord(send_rec);
        }
      }};

  LiveBlocking target = builder_.SetDataset(dataset::kXnasItch)
                            .SetSendTsOut(kTsOut)
                            .SetAddress(kLocalhost, mock_server.Port())
                            .BuildBlocking();
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
  auto mock_server = std::make_unique<mock::MockLsgServer>(
      dataset::kXnasItch, kTsOut, [send_rec, &has_stopped](mock::MockLsgServer& self) {
        self.Accept();
        self.Authenticate();
        self.SendRecord(send_rec);
        while (!has_stopped) {
          std::this_thread::yield();
        }
        const std::string rec_str{reinterpret_cast<const char*>(&send_rec),
                                  sizeof(send_rec)};
        while (self.UncheckedSend(rec_str) == static_cast<::ssize_t>(rec_str.size())) {
        }
      });

  LiveBlocking target = builder_.SetDataset(dataset::kXnasItch)
                            .SetSendTsOut(kTsOut)
                            .SetAddress(kLocalhost, mock_server->Port())
                            .BuildBlocking();
  ASSERT_EQ(target.NextRecord().Get<WithTsOut<TradeMsg>>(), send_rec);
  target.Stop();
  has_stopped = true;
  // kill mock server and join thread before client goes out of scope
  // to ensure Stop is killing the connection, not the client's destructor
  mock_server.reset();
}

TEST_F(LiveBlockingTests, TestConnectWhenGatewayNotUp) {
  builder_.SetDataset(dataset::kXnasItch).SetAddress(kLocalhost, 80);
  ASSERT_THROW(builder_.BuildBlocking(), databento::TcpError);
}

TEST_F(LiveBlockingTests, TestReconnectAndResubscribe) {
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
  auto mock_server = std::make_unique<mock::MockLsgServer>(
      dataset::kXnasItch, kTsOut,
      [kRec, &has_closed, &has_closed_cv, &has_closed_mutex, &should_close,
       &should_close_cv, &should_close_mutex](mock::MockLsgServer& self) {
        self.Accept();
        self.Authenticate();
        self.Subscribe(kAllSymbols, Schema::Trades, SType::RawSymbol, "0", true);
        self.Start();
        self.SendRecord(kRec);
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
        self.Subscribe(kAllSymbols, Schema::Trades, SType::RawSymbol, true);
        self.Start();
        self.SendRecord(kRec);
      });
  LiveBlocking target = builder_.SetDataset(dataset::kXnasItch)
                            .SetSendTsOut(kTsOut)
                            .SetAddress(kLocalhost, mock_server->Port())
                            .BuildBlocking();
  ASSERT_TRUE(target.Subscriptions().empty());
  target.Subscribe(kAllSymbols, Schema::Trades, SType::RawSymbol, "0");
  ASSERT_EQ(target.Subscriptions().size(), 1);
  target.Start();
  const auto rec1 = target.NextRecord();
  ASSERT_TRUE(rec1.Holds<TradeMsg>());
  ASSERT_EQ(rec1.Get<TradeMsg>(), kRec);
  ASSERT_EQ(target.Subscriptions().size(), 1);

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
  target.Resubscribe();
  ASSERT_EQ(target.Subscriptions().size(), 1);
  ASSERT_TRUE(std::holds_alternative<LiveSubscription::NoStart>(
      target.Subscriptions()[0].start));
  const auto metadata = target.Start();
  EXPECT_FALSE(metadata.schema.has_value());
  const auto rec2 = target.NextRecord();
  ASSERT_TRUE(rec2.Holds<TradeMsg>());
  ASSERT_EQ(rec2.Get<TradeMsg>(), kRec);
}
}  // namespace databento::tests
