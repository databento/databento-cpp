#pragma once

#include <cstdint>
#include <sstream>
#include <string>

#include "databento/constants.hpp"
#include "databento/datetime.hpp"

namespace databento::pretty {
// A helper type for formatting the fixed-precision prices used in DBN.
//
// Supports configurable fill, width, and precision [0, 6). By default
// will print all 9 decimal places of precision.
struct Px {
  bool IsUndefined() const { return val == databento::kUndefPrice; }

  std::int64_t val;
};

std::ostream& operator<<(std::ostream& stream, Px px);

// A helper type for formatting the nanosecond UNIX timestamps used in DBN to
// the canonical ISO 8601 format used by Databento.
//
// Supports configurable fill and width.
struct Ts {
  bool IsUndefined() const {
    return val.time_since_epoch().count() == databento::kUndefTimestamp;
  }

  UnixNanos val;
};

std::ostream& operator<<(std::ostream& stream, Ts ts);

// Convert a fixed-precision price to a formatted string.
inline std::string PxToString(std::int64_t px) {
  std::ostringstream ss;
  ss << Px{px};
  return ss.str();
}
}  // namespace databento::pretty
