#pragma once

#include <cstdint>
#include <iosfwd>  // ostream
#include <string>

namespace databento {
enum class LogLevel : std::uint8_t {
  Debug,
  Info,
  Warning,
  Error,
};

class ILogReceiver {
 public:
  static ILogReceiver* Default();

  virtual ~ILogReceiver() = default;

  virtual void Receive(databento::LogLevel level, const std::string& msg) = 0;
  virtual bool ShouldLog(databento::LogLevel) const { return true; }
};

class NullLogReceiver : public ILogReceiver {
 public:
  void Receive(databento::LogLevel, const std::string&) override {}
  bool ShouldLog(databento::LogLevel) const override { return false; }
};

class ConsoleLogReceiver : public ILogReceiver {
 public:
  ConsoleLogReceiver();
  explicit ConsoleLogReceiver(LogLevel min_level);
  explicit ConsoleLogReceiver(std::ostream& stream);
  ConsoleLogReceiver(LogLevel min_level, std::ostream& stream);

  void Receive(LogLevel level, const std::string& msg) override;
  bool ShouldLog(databento::LogLevel level) const override {
    return level >= min_level_;
  }

 private:
  std::ostream& stream_;
  const databento::LogLevel min_level_;
};

std::ostream& operator<<(std::ostream& out, LogLevel level);
const char* ToString(LogLevel level);

void LogPlatformInfo();
void LogPlatformInfo(ILogReceiver* log_receiver);
}  // namespace databento
