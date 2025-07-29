#include <gmock/gmock.h>

#include <sstream>
#include <string>

#include "databento/log.hpp"
#include "databento/system.hpp"
#include "databento/version.hpp"
#include "gmock/gmock.h"
#include "mock/mock_log_receiver.hpp"

namespace databento::tests {
class ConsoleLogReceiverTests : public testing::Test {
 protected:
  std::ostringstream stream_{};
  ConsoleLogReceiver target_{stream_};
};

TEST_F(ConsoleLogReceiverTests, TestOutput) {
  const std::string msg = "Something went wrong";
  target_.Receive(LogLevel::Warning, msg);
  const std::string output = stream_.str();
  // ConsoleLogReceiver adds newline
  ASSERT_EQ("WARN: " + msg + '\n', output);
}

TEST_F(ConsoleLogReceiverTests, TestFilter) {
  const std::string msg = "Something happened";
  target_.Receive(LogLevel::Debug, msg);
  const std::string output = stream_.str();
  EXPECT_TRUE(output.empty());
  target_.Receive(LogLevel::Info, msg);
  EXPECT_THAT(stream_.str(), testing::HasSubstr(msg));
}

TEST(ILogReceiverTests, TestDefault) {
  auto* log_receiver = ILogReceiver::Default();
  ASSERT_NE(log_receiver, nullptr);
  ASSERT_NE(dynamic_cast<ConsoleLogReceiver*>(log_receiver), nullptr);
}

TEST(ILogReceiverTests, TestLogPlatformInfo) {
  mock::MockLogReceiver receiver{[](auto, LogLevel lvl, const std::string& msg) {
    EXPECT_EQ(lvl, LogLevel::Info);
    EXPECT_THAT(msg, testing::HasSubstr(DATABENTO_SYSTEM_ID));
    EXPECT_THAT(msg, testing::HasSubstr(DATABENTO_SYSTEM_VERSION));
    EXPECT_THAT(msg, testing::HasSubstr(DATABENTO_CXX_COMPILER_ID));
    EXPECT_THAT(msg, testing::HasSubstr(DATABENTO_CXX_COMPILER_VERSION));
    EXPECT_THAT(msg, testing::HasSubstr(DATABENTO_VERSION));
  }};
  LogPlatformInfo(&receiver);
  ASSERT_EQ(receiver.CallCount(), 1);
}
}  // namespace databento::tests
