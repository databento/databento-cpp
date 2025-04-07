#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

#include "databento/datetime.hpp"  // UnixNanos
#include "databento/enums.hpp"     // Schema, SType

namespace databento {
struct LiveSubscription {
  struct Snapshot {};
  struct NoStart {};
  using Start = std::variant<Snapshot, UnixNanos, std::string, NoStart>;

  std::vector<std::string> symbols;
  Schema schema;
  SType stype_in;
  Start start;
  std::uint32_t id{};
};
}  // namespace databento
