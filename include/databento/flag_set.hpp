#pragma once

#include <cstdint>
#include <ostream>
#include <string>

namespace databento {
// Transparent wrapper around the bit flags used in several DBN record types.
class FlagSet {
 public:
  using Repr = std::uint8_t;
  // Indicates it's the last message in the event from the venue for a given
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
  // Indicates a publisher-specific event.
  static constexpr Repr kPublisherSpecific = 1 << 1;

  friend std::ostream& operator<<(std::ostream&, FlagSet);

  constexpr FlagSet() : repr_{0} {};

  explicit constexpr FlagSet(Repr repr) : repr_{repr} {}

  explicit constexpr operator std::uint8_t() const { return repr_; }

  constexpr bool operator==(FlagSet rhs) const { return repr_ == rhs.repr_; }
  constexpr bool operator!=(FlagSet rhs) const { return repr_ != rhs.repr_; }

  FlagSet Clear() {
    repr_ = 0;
    return *this;
  }

  constexpr Repr Raw() const { return repr_; }
  void SetRaw(Repr raw) { repr_ = raw; }

  // Checks if any flags are set.
  constexpr bool Any() const { return repr_ != 0; }
  constexpr bool IsEmpty() const { return repr_ == 0; }
  constexpr bool IsLast() const { return bits_.last; }
  FlagSet SetLast() {
    bits_.last = true;
    return *this;
  }
  constexpr bool IsTob() const { return bits_.tob; }
  FlagSet SetTob() {
    bits_.tob = true;
    return *this;
  }
  constexpr bool IsSnapshot() const { return bits_.snapshot; }
  FlagSet SetSnapshot() {
    bits_.snapshot = true;
    return *this;
  }
  constexpr bool IsMbp() const { return bits_.mbp; }
  FlagSet SetMbp() {
    bits_.mbp = true;
    return *this;
  }
  constexpr bool IsBadTsRecv() const { return bits_.bad_ts_recv; }
  FlagSet SetBadTsRecv() {
    bits_.bad_ts_recv = true;
    return *this;
  }
  constexpr bool IsMaybeBadBook() const { return bits_.maybe_bad_book; }
  FlagSet SetMaybeBadBook() {
    bits_.maybe_bad_book = true;
    return *this;
  }
  constexpr bool IsPublisherSpecific() const { return bits_.publisher_specific; }
  FlagSet SetPublisherSpecific() {
    bits_.publisher_specific = true;
    return *this;
  }

 private:
  struct BitFlags {
    bool reserved0 : 1;
    bool publisher_specific : 1;
    bool maybe_bad_book : 1;
    bool bad_ts_recv : 1;
    bool mbp : 1;
    bool snapshot : 1;
    bool tob : 1;
    bool last : 1;
  };
  union {
    BitFlags bits_;
    Repr repr_;
  };
};

std::ostream& operator<<(std::ostream& stream, FlagSet flag_set);
std::string ToString(FlagSet flags);

static_assert(sizeof(FlagSet) == sizeof(std::uint8_t),
              "FlagSet must be a transparent wrapper around std::uint8_t");
}  // namespace databento
