#pragma once

#include <cstdint>
#include <string>

#include "databento/pretty.hpp"

namespace databento {
// Has been renamed to pretty::Px
using FixPx [[deprecated]] = pretty::Px;

// Has been moved to the pretty namespace
[[deprecated]] inline std::string PxToString(std::int64_t px) {
  return pretty::PxToString(px);
}
}  // namespace databento
