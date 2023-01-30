#pragma once

#include <bitset>
#include <cstdint>
#include <ostream>
#include <string>

namespace databento {
/// Transparent wrapper around the bit flags used in several DBN record types.
class FlagSet {
 public:
  using Repr = std::uint8_t;
  // Last message in the packet from the venue for a given `product_id`.
  static constexpr Repr kLast = 1 << 7;
  // Aggregated price level message, not an individual order.
  static constexpr Repr kMbp = 1 << 4;
  // The `ts_recv` value is inaccurate due to clock issues or packet reordering.
  static constexpr Repr kBadTsRecv = 1 << 3;

  friend std::ostream& operator<<(std::ostream&, FlagSet);

  constexpr FlagSet() = default;

  constexpr FlagSet(
      std::uint8_t repr)  // cppcheck-suppress noExplicitConstructor
      : repr_{repr} {}

  constexpr FlagSet operator~() const { return FlagSet(~repr_); }

  constexpr FlagSet operator|(FlagSet rhs) const {
    return FlagSet(repr_ | rhs.repr_);
  }

  constexpr FlagSet operator&(FlagSet rhs) const {
    return FlagSet(repr_ & rhs.repr_);
  }

  constexpr FlagSet operator^(FlagSet rhs) const {
    return FlagSet(repr_ ^ rhs.repr_);
  }

  FlagSet operator|=(FlagSet rhs) {
    repr_ = repr_ | rhs.repr_;
    return *this;
  }

  FlagSet operator&=(FlagSet rhs) {
    repr_ = repr_ & rhs.repr_;
    return *this;
  }

  FlagSet operator^=(FlagSet rhs) {
    repr_ = repr_ ^ rhs.repr_;
    return *this;
  }

  constexpr bool operator==(FlagSet rhs) const { return repr_ == rhs.repr_; }

  constexpr bool operator!=(FlagSet rhs) const { return repr_ != rhs.repr_; }

  // Checks if any flags are set.
  constexpr bool Any() const { return repr_ != 0; }

 private:
  Repr repr_{};
};

inline std::ostream& operator<<(std::ostream& stream, FlagSet flag) {
  // print as binary
  stream << "0b" << std::bitset<8>{flag.repr_};
  return stream;
}

static_assert(sizeof(FlagSet) == sizeof(std::uint8_t),
              "FlagSet must be a transparent wrapper around std::uint8_t");
}  // namespace databento
