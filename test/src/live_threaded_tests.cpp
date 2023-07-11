#include <gtest/gtest.h>

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <exception>
#include <iostream>
#include <memory>
#include <thread>  // this_thread

#include "databento/constants.hpp"
#include "databento/datetime.hpp"
#include "databento/dbn.hpp"
#include "databento/enums.hpp"
#include "databento/exceptions.hpp"
#include "databento/live_threaded.hpp"
#include "databento/log.hpp"
#include "databento/record.hpp"
#include "databento/symbology.hpp"
#include "databento/timeseries.hpp"
#include "gtest/gtest.h"
#include "mock/mock_lsg_server.hpp"

namespace databento {
namespace test {
class LiveThreadedTests : public testing::Test {
 protected:
  template <typename T>
  static constexpr RecordHeader DummyHeader(RType rtype) {
    return {sizeof(T) / RecordHeader::kLengthMultiplier, rtype, 1, 1,
            UnixNanos{}};
  }

  static constexpr auto kKey = "32-character-with-lots-of-filler";
  static constexpr auto kTsOut = false;

  std::unique_ptr<ILogReceiver> logger_{new NullLogReceiver};
};

TEST_F(LiveThreadedTests, TestBasic) {
  const MboMsg kRec{DummyHeader<MboMsg>(RType::Mbo),
                    1,
                    2,
                    3,
                    0,
                    4,
                    Action::Add,
                    Side::Bid,
                    UnixNanos{},
                    TimeDeltaNanos{},
                    100};
  const mock::MockLsgServer mock_server{dataset::kGlbxMdp3, kTsOut,
                                        [&kRec](mock::MockLsgServer& self) {
                                          self.Accept();
                                          self.Authenticate();
                                          self.Start(Schema::Mbo);
                                          self.SendRecord(kRec);
                                          self.SendRecord(kRec);
                                        }};

  LiveThreaded target{logger_.get(),      kKey,
                      dataset::kGlbxMdp3, "127.0.0.1",
                      mock_server.Port(), kTsOut};
  std::atomic<std::uint32_t> call_count{};
  target.Start([&call_count, &kRec](const Record& rec) {
    ++call_count;
    EXPECT_TRUE(rec.Holds<MboMsg>());
    EXPECT_EQ(rec.Get<MboMsg>(), kRec);
    return databento::KeepGoing::Continue;
  });
  while (call_count < 2) {
    std::this_thread::yield();
  }
}

TEST_F(LiveThreadedTests, TestTimeoutRecovery) {
  constexpr auto kSchema = Schema::Ohlcv1M;
  const MboMsg kRec{DummyHeader<MboMsg>(RType::Mbo),
                    1,
                    2,
                    3,
                    0,
                    4,
                    Action::Add,
                    Side::Bid,
                    UnixNanos{},
                    TimeDeltaNanos{},
                    100};
  std::atomic<std::uint32_t> call_count{};
  const mock::MockLsgServer mock_server{
      dataset::kXnasItch, kTsOut,
      [&kRec, &call_count](mock::MockLsgServer& self) {
        self.Accept();
        self.Authenticate();
        self.Start(kSchema);
        self.SendRecord(kRec);
        while (call_count < 1) {
          std::this_thread::yield();
        }
        // 150% of live threaded timeout
        std::this_thread::sleep_for(std::chrono::milliseconds{75});
        self.SendRecord(kRec);
      }};

  LiveThreaded target{logger_.get(),      kKey, dataset::kXnasItch, "127.0.0.1",
                      mock_server.Port(), false};
  target.Start(
      [kSchema](Metadata&& metadata) { EXPECT_EQ(metadata.schema, kSchema); },
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
  constexpr auto kSchema = Schema::Ohlcv1M;
  const MboMsg kRec{DummyHeader<MboMsg>(RType::Mbo),
                    1,
                    2,
                    3,
                    0,
                    4,
                    Action::Add,
                    Side::Bid,
                    UnixNanos{},
                    TimeDeltaNanos{},
                    100};
  std::atomic<std::uint32_t> call_count{};
  std::unique_ptr<mock::MockLsgServer> mock_server{new mock::MockLsgServer{
      dataset::kXnasItch, kTsOut,
      [&kRec, &call_count](mock::MockLsgServer& self) {
        self.Accept();
        self.Authenticate();
        self.Start(kSchema);
        self.SendRecord(kRec);
        self.SendRecord(kRec);
        while (call_count < 1) {
          std::this_thread::yield();
        }
        const std::string rec_str{reinterpret_cast<const char*>(&kRec),
                                  sizeof(kRec)};
        while (self.UncheckedSend(rec_str) ==
               static_cast<::ssize_t>(rec_str.size())) {
          std::this_thread::yield();
        }
      }}};

  LiveThreaded target{logger_.get(),       kKey,
                      dataset::kXnasItch,  "127.0.0.1",
                      mock_server->Port(), kTsOut};
  target.Start(
      [kSchema](Metadata&& metadata) { EXPECT_EQ(metadata.schema, kSchema); },
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

TEST_F(LiveThreadedTests, TestExceptionCallbackAndReconnect) {
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

  bool should_close{};
  std::mutex should_close_mutex;
  std::condition_variable should_close_cv;
  const mock::MockLsgServer mock_server{
      dataset::kXnasItch, kTsOut,
      [&should_close, &should_close_mutex, &should_close_cv,
       kRec](mock::MockLsgServer& self) {
        self.Accept();
        self.Authenticate();
        self.Subscribe(kAllSymbols, kSchema, kSType);
        self.Start(kSchema);
        {
          std::unique_lock<std::mutex> shutdown_lock{should_close_mutex};
          should_close_cv.wait(shutdown_lock,
                               [&should_close] { return should_close; });
        }
        self.Close();
        self.Accept();
        self.Authenticate();
        self.Subscribe(kAllSymbols, kSchema, kSType);
        self.Start(kSchema);
        self.SendRecord(kRec);
      }};
  LiveThreaded target{logger_.get(),      kKey,
                      dataset::kXnasItch, "127.0.0.1",
                      mock_server.Port(), kTsOut};
  std::atomic<std::int32_t> metadata_calls{};
  const auto metadata_cb = [kSchema, &metadata_calls, &should_close,
                            &should_close_cv,
                            &should_close_mutex](Metadata&& metadata) {
    ++metadata_calls;
    EXPECT_EQ(metadata.schema, kSchema);
    // close server
    const std::lock_guard<std::mutex> _lock{should_close_mutex};
    should_close = true;
    should_close_cv.notify_one();
  };
  std::atomic<std::int32_t> record_calls{};
  const auto record_cb = [&record_calls, kRec](const Record& record) {
    ++record_calls;
    EXPECT_EQ(record.Get<TradeMsg>(), kRec);
    return KeepGoing::Continue;
  };
  std::atomic<std::int32_t> exception_calls{};
  const auto exception_cb = [&exception_calls,
                             &target](const std::exception& exc) {
    ++exception_calls;
    if (exception_calls == 1) {
      EXPECT_NE(dynamic_cast<const databento::DbnResponseError*>(&exc), nullptr)
          << "Unexpected exception type";
      target.Reconnect();
      target.Subscribe(kAllSymbols, kSchema, kSType);
      return LiveThreaded::ExceptionAction::Restart;
    } else {
      GTEST_NONFATAL_FAILURE_("Exception callback called more than expected");
      return LiveThreaded::ExceptionAction::Stop;
    }
  };
  target.Subscribe(kAllSymbols, kSchema, kSType);
  target.Start(metadata_cb, record_cb, exception_cb);
  while (exception_calls == 0 || record_calls == 0) {
    std::this_thread::yield();
  }
  EXPECT_EQ(metadata_calls, 2);
  EXPECT_EQ(exception_calls, 1);
  EXPECT_EQ(record_calls, 1);
}

TEST_F(LiveThreadedTests, TestDeadlockPrevention) {
  constexpr auto kSchema = Schema::Trades;
  constexpr auto kSType = SType::Parent;
  const std::vector<std::string> kSymbols = {"LO.OPT", "6E.FUT"};

  bool should_close{};
  std::mutex should_close_mutex;
  std::condition_variable should_close_cv;
  testing::internal::CaptureStderr();
  const mock::MockLsgServer mock_server{
      dataset::kXnasItch, kTsOut,
      [&kSymbols, &should_close, &should_close_mutex,
       &should_close_cv](mock::MockLsgServer& self) {
        self.Accept();
        self.Authenticate();
        self.Start(kSchema);
        {
          std::unique_lock<std::mutex> shutdown_lock{should_close_mutex};
          should_close_cv.wait(shutdown_lock,
                               [&should_close] { return should_close; });
        }
        self.Close();
        self.Accept();
        self.Authenticate();
        self.Subscribe(kSymbols, kSchema, kSType);
      }};
  LiveThreaded target{ILogReceiver::Default(), kKey,
                      dataset::kXnasItch,      "127.0.0.1",
                      mock_server.Port(),      kTsOut};
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
  std::atomic<std::int32_t> exception_calls{};
  const auto exception_cb = [&exception_calls, &target, &metadata_cb,
                             &record_cb, &kSymbols](const std::exception& exc) {
    EXPECT_NE(dynamic_cast<const databento::DbnResponseError*>(&exc), nullptr)
        << "Unexpected exception type";
    target.Reconnect();
    target.Subscribe(kSymbols, kSchema, kSType);
    // Not supposed to do this
    target.Start(metadata_cb, record_cb, [](const std::exception&) {
      GTEST_NONFATAL_FAILURE_("Unexpectedly called exception callback");
      return LiveThreaded::ExceptionAction::Stop;
    });
    ++exception_calls;
    return LiveThreaded::ExceptionAction::Stop;
  };
  target.Start(metadata_cb, record_cb, exception_cb);
  while (exception_calls == 0) {
    std::this_thread::yield();
  }
  std::clog.flush();
  const std::string output = testing::internal::GetCapturedStderr();
  EXPECT_NE(output.find("which would cause a deadlock"), std::string::npos)
      << "Got unexpected output: " << output;
}
}  // namespace test
}  // namespace databento
