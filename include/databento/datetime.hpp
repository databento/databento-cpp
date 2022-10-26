#pragma once

#include <chrono>
#include <cstdint>
#include <ratio>  // nano
#include <string>

namespace databento {
using UnixNanos =
    std::chrono::time_point<std::chrono::system_clock,
                            std::chrono::duration<uint64_t, std::nano>>;
using TimeDeltaNanos = std::chrono::duration<int32_t, std::nano>;
std::string ToString(UnixNanos epoch_nanos);
// Converts a YYYYMMDD integer to a YYYY-MM-DD string.
std::string DateFromIso8601Int(std::uint32_t date_int);
}  // namespace databento
