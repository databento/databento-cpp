#pragma once

#include <chrono>
#include <cstdint>
#include <ratio>  // nano
#include <string>
#include <utility>

namespace databento {
// Nanoseconds since the UNIX epoch.
using UnixNanos = std::chrono::time_point<std::chrono::system_clock,
                                          std::chrono::duration<uint64_t, std::nano>>;
// A representation of the difference between two timestamps.
using TimeDeltaNanos = std::chrono::duration<int32_t, std::nano>;
std::string ToString(UnixNanos unix_nanos);
// Format the UNIX timestamp as a human-readable ISO8601 string of format
// YYYY-MM-DDTHH:MM:SS.fffffffffZ
std::string ToIso8601(UnixNanos unix_nanos);
std::string ToString(TimeDeltaNanos td_nanos);
// Converts a YYYYMMDD integer to a YYYY-MM-DD string.
std::string DateFromIso8601Int(std::uint32_t date_int);

template <typename T>
struct DateTimeRange {
  explicit DateTimeRange(T start_) : DateTimeRange{std::move(start_), {}} {}
  // underscore to prevent shadowing
  DateTimeRange(T start_, T end_) : start{std::move(start_)}, end{std::move(end_)} {}

  T start;
  T end;
};
using DateRange = DateTimeRange<std::string>;

template <typename T>
inline bool operator==(const DateTimeRange<T>& lhs, const DateTimeRange<T>& rhs) {
  return lhs.start == rhs.start && lhs.end == rhs.end;
}
template <typename T>
inline bool operator!=(const DateTimeRange<T>& lhs, const DateTimeRange<T>& rhs) {
  return !(lhs == rhs);
}

std::string ToString(const DateTimeRange<std::string>& dt_range);
std::ostream& operator<<(std::ostream& stream,
                         const DateTimeRange<std::string>& dt_range);
std::string ToString(const DateTimeRange<UnixNanos>& dt_range);
std::ostream& operator<<(std::ostream& stream,
                         const DateTimeRange<UnixNanos>& dt_range);
}  // namespace databento
