#include <gtest/gtest.h>

#include <iostream>
#include <string>

#include "databento/log.hpp"

namespace databento {
namespace test {
class ConsoleLogReceiverTests : public testing::Test {
 protected:
  ConsoleLogReceiver target_{};
};

TEST_F(ConsoleLogReceiverTests, TestOutput) {
  testing::internal::CaptureStderr();
  const std::string msg = "Something went wrong";
  target_.Receive(LogLevel::Warning, msg);
  std::clog.flush();
  const std::string output = testing::internal::GetCapturedStderr();
  // ConsoleLogReceiver adds newline
  ASSERT_EQ(msg + '\n', output);
}

TEST_F(ConsoleLogReceiverTests, TestFilter) {
  testing::internal::CaptureStderr();
  const std::string msg = "Something happened";
  target_.Receive(LogLevel::Debug, msg);
  std::clog.flush();
  const std::string output = testing::internal::GetCapturedStderr();
  ASSERT_TRUE(output.empty());
}

TEST(ILogReceiverTests, TestDefault) {
  auto* log_receiver = ILogReceiver::Default();
  ASSERT_NE(log_receiver, nullptr);
  ASSERT_NE(dynamic_cast<ConsoleLogReceiver*>(log_receiver), nullptr);
  testing::internal::CaptureStderr();
  const std::string msg = "Fatal error";
  log_receiver->Receive(LogLevel::Error, msg);
  const std::string output = testing::internal::GetCapturedStderr();
  ASSERT_EQ(msg + '\n', output);
}
}  // namespace test
}  // namespace databento
