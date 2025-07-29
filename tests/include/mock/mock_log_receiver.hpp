#pragma once

#include <gtest/gtest.h>

#include <cstddef>
#include <functional>
#include <utility>

#include "databento/log.hpp"

namespace databento::tests::mock {
class MockLogReceiver : public databento::ILogReceiver {
 public:
  using LogCallback =
      std::function<void(std::size_t, databento::LogLevel, const std::string&)>;

  explicit MockLogReceiver(LogCallback callback)
      : MockLogReceiver{LogLevel::Info, std::move(callback)} {}
  MockLogReceiver(LogLevel min_level, LogCallback callback)
      : callback_{std::move(callback)}, min_level_{min_level} {}

  static MockLogReceiver AssertNoLogs(databento::LogLevel min_level) {
    return MockLogReceiver{min_level, [min_level](auto, databento::LogLevel level,
                                                  const std::string& msg) {
                             FAIL() << "Received unexpected log message with level "
                                    << level << ": " << msg;
                           }};
  }

  void Receive(databento::LogLevel level, const std::string& msg) override {
    if (level >= min_level_) {
      callback_(call_count_, level, msg);
      ++call_count_;
    }
  }

  bool ShouldLog(databento::LogLevel level) const override {
    return level >= min_level_;
  }

  std::size_t CallCount() const { return call_count_; }

 private:
  LogCallback callback_;
  LogLevel min_level_{};
  std::size_t call_count_{};
};
}  // namespace databento::tests::mock
