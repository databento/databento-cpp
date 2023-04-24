#pragma once

#include <cstdint>
#include <sstream>
#include <string>

#include "databento/constants.hpp"

namespace databento {
// A fixed-precision price.
struct FixPx {
  bool IsUndefined() const { return val == databento::kUndefPrice; }

  std::int64_t val;
};

std::ostream& operator<<(std::ostream& stream, FixPx fix_px);

// Convert a fixed-precision price to a formatted string.
inline std::string PxToString(std::int64_t px) {
  std::ostringstream ss;
  ss << FixPx{px};
  return ss.str();
}
}  // namespace databento
