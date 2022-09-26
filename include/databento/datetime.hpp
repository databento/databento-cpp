#pragma once

#include <chrono>

namespace databento {
using EpochNanos = std::chrono::time_point<std::chrono::system_clock,
                                           std::chrono::nanoseconds>;
}
