#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <exception>
#include <memory>
#include <thread>  // this_thread
#include <variant>

#include "databento/constants.hpp"
#include "databento/datetime.hpp"
#include "databento/dbn.hpp"
#include "databento/enums.hpp"
#include "databento/exceptions.hpp"
#include "databento/live.hpp"
#include "databento/live_subscription.hpp"
#include "databento/live_threaded.hpp"
#include "databento/log.hpp"
#include "databento/record.hpp"
#include "databento/symbology.hpp"
#include "databento/timeseries.hpp"
#include "mock/mock_log_receiver.hpp"
#include "mock/mock_lsg_server.hpp"

namespace databento::tests {
class LiveThreadedTests : public testing::Test {
 protected:
  template <typename T>
  static constexpr RecordHeader DummyHeader(RType rtype) {
    return {sizeof(T) / RecordHeader::kLengthMultiplier, rtype, 1, 1, UnixNanos{}};
  }

  static constexpr auto kKey = "32-character-with-lots-of-filler";
  static constexpr auto kTsOut = false;
  static constexpr auto kLocalhost = "127.0.0.1";

  mock::MockLogReceiver logger_ =
      mock::MockLogReceiver::AssertNoLogs(LogLevel::Warning);
  LiveBuilder builder_{LiveBuilder{}.SetLogReceiver(&logger_).SetKey(kKey)};
};

TEST_F(LiveThreadedTests, TestBasic) {
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
  constexpr auto kHeartbeatInterval = std::chrono::seconds{5};
  const mock::MockLsgServer mock_server{dataset::kGlbxMdp3, kTsOut, kHeartbeatInterval,
                                        [&kRec](mock::MockLsgServer& self) {
                                          self.Accept();
                                          self.Authenticate();
                                          self.Start();
                                          self.SendRecord(kRec);
                                          self.SendRecord(kRec);
                                        }};

  LiveThreaded target = builder_.SetDataset(dataset::kGlbxMdp3)
                            .SetSendTsOut(kTsOut)
                            .SetHeartbeatInterval(kHeartbeatInterval)
                            .SetAddress(kLocalhost, mock_server.Port())
                            .BuildThreaded();
  std::uint32_t call_count{};
  target.Start([&call_count, &kRec](const Record& rec) {
    ++call_count;
    EXPECT_TRUE(rec.Holds<MboMsg>());
    EXPECT_EQ(rec.Get<MboMsg>(), kRec);
    return call_count < 2 ? KeepGoing::Continue : KeepGoing::Stop;
  });
  target.BlockForStop();
}

TEST_F(LiveThreadedTests, TestTimeoutRecovery) {
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
  std::atomic<std::uint32_t> call_count{};
  const mock::MockLsgServer mock_server{
      dataset::kXnasItch, kTsOut, [&kRec, &call_count](mock::MockLsgServer& self) {
        self.Accept();
        self.Authenticate();
        self.Start();
        self.SendRecord(kRec);
        while (call_count < 1) {
          std::this_thread::yield();
        }
        // 150% of live threaded timeout
        std::this_thread::sleep_for(std::chrono::milliseconds{75});
        self.SendRecord(kRec);
      }};

  LiveThreaded target = builder_.SetDataset(dataset::kXnasItch)
                            .SetSendTsOut(kTsOut)
                            .SetAddress(kLocalhost, mock_server.Port())
                            .BuildThreaded();
  target.Start([](Metadata&& metadata) { EXPECT_FALSE(metadata.schema.has_value()); },
               [&call_count, &kRec](const Record& rec) {
                 ++call_count;
                 EXPECT_TRUE(rec.Holds<MboMsg>());
                 EXPECT_EQ(rec.Get<MboMsg>(), kRec);
                 return databento::KeepGoing::Continue;
               });
  while (call_count < 2) {
    std::this_thread::yield();
  }
}

TEST_F(LiveThreadedTests, TestStop) {
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
  std::atomic<std::uint32_t> call_count{};
  auto mock_server = std::make_unique<mock::MockLsgServer>(
      dataset::kXnasItch, kTsOut, [&kRec, &call_count](mock::MockLsgServer& self) {
        self.Accept();
        self.Authenticate();
        self.Start();
        self.SendRecord(kRec);
        self.SendRecord(kRec);
        while (call_count < 1) {
          std::this_thread::yield();
        }
        const std::string rec_str{reinterpret_cast<const char*>(&kRec), sizeof(kRec)};
        while (self.UncheckedSend(rec_str) == static_cast<::ssize_t>(rec_str.size())) {
          std::this_thread::yield();
        }
      });

  LiveThreaded target = builder_.SetDataset(dataset::kXnasItch)
                            .SetSendTsOut(kTsOut)
                            .SetAddress(kLocalhost, mock_server->Port())
                            .BuildThreaded();
  target.Start([](Metadata&& metadata) { EXPECT_FALSE(metadata.schema.has_value()); },
               [&call_count, &kRec](const Record& rec) {
                 ++call_count;
                 EXPECT_EQ(call_count, 1) << "Record callback called more than once";
                 EXPECT_TRUE(rec.Holds<MboMsg>());
                 EXPECT_EQ(rec.Get<MboMsg>(), kRec);
                 return databento::KeepGoing::Stop;
               });
  // kill mock server and join thread before client goes out of scope
  // to ensure Stop is killing the connection, not the client's destructor
  mock_server.reset();
}

TEST_F(LiveThreadedTests, TestExceptionCallbackReconnectAndResubscribe) {
  constexpr auto kSchema = Schema::Trades;
  constexpr auto kSType = SType::RawSymbol;
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

  const auto kUseSnapshot = true;
  bool should_close{};
  std::mutex should_close_mutex;
  std::condition_variable should_close_cv;
  const mock::MockLsgServer mock_server{
      dataset::kXnasItch, kTsOut,
      [&should_close, &should_close_mutex, &should_close_cv, kRec, kSchema, kSType,
       kUseSnapshot](mock::MockLsgServer& self) {
        self.Accept();
        self.Authenticate();
        self.Subscribe(kAllSymbols, kSchema, kSType, "0", true);
        self.Start();
        self.SendRecord(kRec);
        {
          std::unique_lock<std::mutex> shutdown_lock{should_close_mutex};
          should_close_cv.wait(shutdown_lock, [&should_close] { return should_close; });
        }
        self.Close();
        self.Accept();
        self.Authenticate();
        self.Subscribe(kAllSymbols, kSchema, kSType, true);
        self.Start();
        self.SendRecord(kRec);
      }};
  logger_ = mock::MockLogReceiver{
      LogLevel::Warning,
      [](auto count, databento::LogLevel level, const std::string& msg) {
        EXPECT_THAT(msg,
                    testing::EndsWith(
                        "Gateway closed the session. Attempting to restart session."));
      }};
  LiveThreaded target = builder_.SetDataset(dataset::kXnasItch)
                            .SetSendTsOut(kTsOut)
                            .SetAddress(kLocalhost, mock_server.Port())
                            .BuildThreaded();
  std::atomic<std::int32_t> metadata_calls{};
  const auto metadata_cb = [&metadata_calls](Metadata&& metadata) {
    ++metadata_calls;
    EXPECT_FALSE(metadata.schema.has_value());
  };
  std::atomic<std::int32_t> record_calls{};
  const auto record_cb = [&record_calls, kRec, &should_close_mutex, &should_close,
                          &should_close_cv](const Record& record) {
    ++record_calls;
    EXPECT_EQ(record.Get<TradeMsg>(), kRec);
    if (record_calls == 1) {  // close server
      const std::lock_guard<std::mutex> _lock{should_close_mutex};
      should_close = true;
      should_close_cv.notify_one();
      return KeepGoing::Continue;
    }
    return KeepGoing::Stop;
  };
  std::atomic<std::int32_t> exception_calls{};
  const auto exception_cb = [&exception_calls, &target, kSchema,
                             kSType](const std::exception& exc) {
    ++exception_calls;
    if (exception_calls == 1) {
      EXPECT_NE(dynamic_cast<const databento::DbnResponseError*>(&exc), nullptr)
          << "Unexpected exception type";
      target.Reconnect();
      target.Resubscribe();
      EXPECT_EQ(target.Subscriptions().size(), 1);
      EXPECT_TRUE(std::holds_alternative<LiveSubscription::NoStart>(
          target.Subscriptions()[0].start));
      return LiveThreaded::ExceptionAction::Restart;
    } else {
      GTEST_NONFATAL_FAILURE_("Exception callback called more than expected");
      return LiveThreaded::ExceptionAction::Stop;
    }
  };

  ASSERT_TRUE(target.Subscriptions().empty());
  target.Subscribe(kAllSymbols, kSchema, kSType, "0");
  ASSERT_EQ(target.Subscriptions().size(), 1);
  target.Start(metadata_cb, record_cb, exception_cb);
  target.BlockForStop();
  EXPECT_EQ(metadata_calls, 2);
  EXPECT_EQ(exception_calls, 1);
  EXPECT_EQ(record_calls, 2);
  EXPECT_EQ(logger_.CallCount(), 1);
}

TEST_F(LiveThreadedTests, TestDeadlockPrevention) {
  const auto kSchema = Schema::Trades;
  const auto kSType = SType::Parent;
  const std::vector<std::string> kSymbols = {"LO.OPT", "6E.FUT"};

  bool should_close{};
  std::mutex should_close_mutex;
  std::condition_variable should_close_cv;
  logger_ = mock::MockLogReceiver{
      LogLevel::Warning,
      [](auto count, databento::LogLevel level, const std::string& msg) {
        if (count == 0) {
          EXPECT_THAT(msg, testing::HasSubstr("which would cause a deadlock"))
              << "Got unexpected log message " << level << ": " << msg;
        }
      }};
  const mock::MockLsgServer mock_server{
      dataset::kXnasItch, kTsOut,
      [&kSymbols, &should_close, &should_close_mutex, &should_close_cv, kSchema,
       kSType](mock::MockLsgServer& self) {
        self.Accept();
        self.Authenticate();
        self.Start();
        {
          std::unique_lock<std::mutex> shutdown_lock{should_close_mutex};
          should_close_cv.wait(shutdown_lock, [&should_close] { return should_close; });
        }
        self.Close();
        self.Accept();
        self.Authenticate();
        self.Subscribe(kSymbols, kSchema, kSType, true);
      }};
  LiveThreaded target = builder_.SetDataset(dataset::kXnasItch)
                            .SetSendTsOut(kTsOut)
                            .SetAddress(kLocalhost, mock_server.Port())
                            .BuildThreaded();
  std::atomic<std::int32_t> metadata_calls{};
  const auto metadata_cb = [&metadata_calls, &should_close, &should_close_cv,
                            &should_close_mutex](Metadata&&) {
    ++metadata_calls;
    // close server
    const std::lock_guard<std::mutex> _lock{should_close_mutex};
    should_close = true;
    should_close_cv.notify_one();
  };
  std::atomic<std::int32_t> record_calls{};
  const auto record_cb = [&record_calls](const Record&) {
    ++record_calls;
    return KeepGoing::Continue;
  };
  const auto exception_cb = [&target, &metadata_cb, &record_cb, &kSymbols, kSchema,
                             kSType](const std::exception& exc) {
    EXPECT_NE(dynamic_cast<const databento::DbnResponseError*>(&exc), nullptr)
        << "Unexpected exception type";
    target.Reconnect();
    target.Subscribe(kSymbols, kSchema, kSType);
    // Not supposed to do this
    target.Start(metadata_cb, record_cb, [](const std::exception&) {
      GTEST_NONFATAL_FAILURE_("Unexpectedly called exception callback");
      return LiveThreaded::ExceptionAction::Stop;
    });
    return LiveThreaded::ExceptionAction::Stop;
  };
  target.Start(metadata_cb, record_cb, exception_cb);
  target.BlockForStop();
  EXPECT_GE(logger_.CallCount(), 1);
}

TEST_F(LiveThreadedTests, TestBlockForStopTimeout) {
  constexpr OhlcvMsg kRec{DummyHeader<OhlcvMsg>(RType::Ohlcv1S), 1, 2, 3, 4, 5};
  const mock::MockLsgServer mock_server{dataset::kXnasItch, kTsOut,
                                        [&kRec](mock::MockLsgServer& self) {
                                          self.Accept();
                                          self.Authenticate();
                                          self.Start();
                                          self.SendRecord(kRec);
                                        }};
  LiveThreaded target = builder_.SetDataset(dataset::kXnasItch)
                            .SetSendTsOut(kTsOut)
                            .SetAddress(kLocalhost, mock_server.Port())
                            .BuildThreaded();
  target.Start([](const Record&) { return KeepGoing::Continue; });
  ASSERT_EQ(target.BlockForStop(std::chrono::milliseconds{100}), KeepGoing::Continue);
}
}  // namespace databento::tests
