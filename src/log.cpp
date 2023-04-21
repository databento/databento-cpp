#include "databento/log.hpp"

#include <iostream>
#include <memory>

databento::ILogReceiver* databento::ILogReceiver::Default() {
  static const std::unique_ptr<ILogReceiver> gDefaultLogger{
      new ConsoleLogReceiver{}};
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
  if (level >= min_level_) {
    stream_ << msg << '\n';
  }
}
