#pragma once

#include <bitset>
#include <cstdint>
#include <ostream>

namespace databento {
// Transparent wrapper around the bit flags used in several DBN record types.
class FlagSet {
 public:
  using Repr = std::uint8_t;
  // Indicates it's the last message in the packet from the venue for a given
  // `instrument_id`.
  static constexpr Repr kLast = 1 << 7;
  // Indicates a top-of-book message, not an individual order.
  static constexpr Repr kTob = 1 << 6;
  // Indicates the message was sourced from a replay, such as a snapshot
  // server.
  static constexpr Repr kSnapshot = 1 << 5;
  // Indicates an aggregated price level message, not an individual order.
  static constexpr Repr kMbp = 1 << 4;
  // Indicates the `ts_recv` value is inaccurate due to clock issues or packet
  // reordering.
  static constexpr Repr kBadTsRecv = 1 << 3;
  // Indicates an unrecoverable gap was detected in the channel.
  static constexpr Repr kMaybeBadBook = 1 << 2;

  friend std::ostream& operator<<(std::ostream&, FlagSet);

  constexpr FlagSet() = default;

  constexpr FlagSet(  // cppcheck-suppress noExplicitConstructor
      std::uint8_t repr)
      : repr_{repr} {}

  explicit constexpr operator std::uint8_t() const { return repr_; }

  constexpr FlagSet operator~() const {
    return FlagSet{static_cast<Repr>(~repr_)};
  }

  constexpr FlagSet operator|(FlagSet rhs) const {
    return FlagSet{static_cast<Repr>(repr_ | rhs.repr_)};
  }

  constexpr FlagSet operator&(FlagSet rhs) const {
    return FlagSet{static_cast<Repr>(repr_ & rhs.repr_)};
  }

  constexpr FlagSet operator^(FlagSet rhs) const {
    return FlagSet{static_cast<Repr>(repr_ ^ rhs.repr_)};
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
