#include <gtest/gtest.h>

#include <sstream>
#include <string>

#include "databento/log.hpp"

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
  ASSERT_EQ(msg + '\n', output);
}

TEST_F(ConsoleLogReceiverTests, TestFilter) {
  const std::string msg = "Something happened";
  target_.Receive(LogLevel::Debug, msg);
  const std::string output = stream_.str();
  ASSERT_TRUE(output.empty());
}

TEST(ILogReceiverTests, TestDefault) {
  auto* log_receiver = ILogReceiver::Default();
  ASSERT_NE(log_receiver, nullptr);
  ASSERT_NE(dynamic_cast<ConsoleLogReceiver*>(log_receiver), nullptr);
}
}  // namespace databento::tests
