#include "databento/datetime.hpp"

// NOLINTNEXTLINE(modernize-deprecated-headers): no thread-safe version in STL
#include <time.h>  // gmtime_r or gmtime_s

#include <array>
#include <chrono>
#include <ctime>    // localtime, strftime, tm
#include <iomanip>  // setw
#include <sstream>  // ostringstream

#include "databento/constants.hpp"  // kUndefTimestamp
#include "stream_op_helper.hpp"

namespace databento {
std::string ToIso8601(UnixNanos unix_nanos) {
  if (unix_nanos.time_since_epoch().count() == kUndefTimestamp) {
    return "UNDEF_TIMESTAMP";
  }
  std::array<char, 80> buf{};
  const std::time_t time =
      std::chrono::duration_cast<std::chrono::seconds>(unix_nanos.time_since_epoch())
          .count();
  std::tm tm = {};
#ifdef _WIN32
  if (::gmtime_s(&tm, &time) != 0) {
    // Fallback on printing nanos
    return ToString(unix_nanos);
  }
#else
  if (::gmtime_r(&time, &tm) == nullptr) {
    // Fallback on printing nanos
    return ToString(unix_nanos);
  }
#endif
  const auto nanos =
      std::chrono::nanoseconds{unix_nanos.time_since_epoch() %
                               std::chrono::nanoseconds{std::chrono::seconds{1}}};
  const size_t count = std::strftime(buf.data(), sizeof(buf), "%Y-%m-%dT%H:%M:%S", &tm);
  std::ostringstream time_ss;
  time_ss.write(buf.data(), static_cast<std::streamsize>(count));
  time_ss << '.' << std::setw(9) << std::setfill('0') << nanos.count() << 'Z';
  return time_ss.str();
}

std::string ToString(UnixNanos unix_nanos) {
  return std::to_string(unix_nanos.time_since_epoch().count());
}

std::string ToString(TimeDeltaNanos td_nanos) {
  return std::to_string(td_nanos.count());
}

std::string DateFromIso8601Int(std::uint32_t date_int) {
  const auto year = date_int / 10000;
  const auto remaining = date_int % 10000;
  const auto month = remaining / 100;
  const auto day = remaining % 100;
  std::ostringstream out_ss;
  out_ss << year << '-' << std::setfill('0') << std::setw(2) << month << '-'
         << std::setfill('0') << std::setw(2) << day;
  return out_ss.str();
}

std::string ToString(const DateTimeRange<std::string>& dt_range) {
  return MakeString(dt_range);
}
std::ostream& operator<<(std::ostream& stream,
                         const DateTimeRange<std::string>& dt_range) {
  return StreamOpBuilder{stream}
      .SetSpacer(" ")
      .SetTypeName("DateTimeRange")
      .Build()
      .AddField("start", dt_range.start)
      .AddField("end", dt_range.end)
      .Finish();
}

std::string ToString(const DateTimeRange<UnixNanos>& dt_range) {
  return MakeString(dt_range);
}
std::ostream& operator<<(std::ostream& stream,
                         const DateTimeRange<UnixNanos>& dt_range) {
  return StreamOpBuilder{stream}
      .SetSpacer(" ")
      .SetTypeName("DateTimeRange")
      .Build()
      .AddField("start", dt_range.start)
      .AddField("end", dt_range.end)
      .Finish();
}
}  // namespace databento
