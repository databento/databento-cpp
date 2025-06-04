#include "databento/log.hpp"

#include <iostream>
#include <memory>

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
    stream_ << level << ": " << msg << '\n';
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
}  // namespace databento
