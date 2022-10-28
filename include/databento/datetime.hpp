#pragma once

#include <chrono>
#include <cstdint>
#include <ratio>  // nano
#include <string>

namespace databento {
// Nanoseconds since the UNIX epoch.
using UnixNanos =
    std::chrono::time_point<std::chrono::system_clock,
                            std::chrono::duration<uint64_t, std::nano>>;
// A representation of the difference between two timestamps.
using TimeDeltaNanos = std::chrono::duration<int32_t, std::nano>;
std::string ToString(UnixNanos unix_nanos);
std::string ToString(TimeDeltaNanos td_nanos);
// Converts a YYYYMMDD integer to a YYYY-MM-DD string.
std::string DateFromIso8601Int(std::uint32_t date_int);
}  // namespace databento
