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

  // The timestamp when the subscription request was made.
  UnixNanos sent_at;
};

struct LiveUnsubscription {
  std::vector<std::string> symbols;
  Schema schema;
  SType stype_in;

  // The timestamp when the unsubscribe request was made.
  UnixNanos sent_at;
};
}  // namespace databento
