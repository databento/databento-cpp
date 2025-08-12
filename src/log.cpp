#include "databento/log.hpp"

#include <iostream>
#include <memory>
#include <sstream>

#include "databento/system.hpp"
#include "databento/version.hpp"
#include "stream_op_helper.hpp"

databento::ILogReceiver* databento::ILogReceiver::Default() {
  static const std::unique_ptr<ILogReceiver> gDefaultLogger{
      std::make_unique<ConsoleLogReceiver>()};
  return gDefaultLogger.get();
}

using databento::ConsoleLogReceiver;

ConsoleLogReceiver::ConsoleLogReceiver()
    : ConsoleLogReceiver{LogLevel::Info, std::clog} {}
ConsoleLogReceiver::ConsoleLogReceiver(LogLevel min_level)
    : ConsoleLogReceiver{min_level, std::clog} {}
ConsoleLogReceiver::ConsoleLogReceiver(std::ostream& stream)
    : ConsoleLogReceiver{LogLevel::Info, stream} {}
ConsoleLogReceiver::ConsoleLogReceiver(LogLevel min_level, std::ostream& stream)
    : stream_{stream}, min_level_{min_level} {}

void ConsoleLogReceiver::Receive(LogLevel level, const std::string& msg) {
  if (ShouldLog(level)) {
    stream_ << level << ": " << msg;
    // Don't add a newline if `msg` ends in one
    if (msg.empty() || msg.back() != '\n') {
      stream_ << '\n';
    }
  }
}

namespace databento {
std::ostream& operator<<(std::ostream& out, LogLevel level) {
  out << ToString(level);
  return out;
}
const char* ToString(LogLevel level) {
  switch (level) {
    case LogLevel::Debug: {
      return "DEBUG";
    }
    case LogLevel::Info: {
      return "INFO";
    }
    case LogLevel::Warning: {
      return "WARN";
    }
    case LogLevel::Error: {
      return "ERROR";
    }
    default: {
      return "UNKNOWN";
    };
  }
}

void LogPlatformInfo() { LogPlatformInfo(ILogReceiver::Default()); }

void LogPlatformInfo(ILogReceiver* log_receiver) {
  std::ostringstream ss;
  StreamOpBuilder{ss}
      .SetSpacer(" ")
      .Build()
      .AddField("client_version", DATABENTO_VERSION)
      .AddField("compiler", DATABENTO_CXX_COMPILER_ID)
      .AddField("compiler_version", DATABENTO_CXX_COMPILER_VERSION)
      .AddField("os", DATABENTO_SYSTEM_ID)
      .AddField("os_version", DATABENTO_SYSTEM_VERSION)
      .Finish();
  log_receiver->Receive(LogLevel::Info, ss.str());
}
}  // namespace databento
