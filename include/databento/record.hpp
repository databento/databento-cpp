#pragma once

#include <array>
#include <chrono>  // nanoseconds
#include <cstddef>
#include <cstdint>

#include "databento/datetime.hpp"  // UnixNanos
#include "databento/enums.hpp"

namespace databento {
// Common data for all Databento Records.
struct RecordHeader {
  // The length of the message in 32-bit words.
  std::uint8_t length;
  // The record type.
  std::uint8_t rtype;
  // The publisher ID assigned by Databento.
  std::uint16_t publisher_id;
  // The product ID assigned by the venue.
  std::uint32_t product_id;
  // The exchange timestamp in UNIX epoch nanoseconds.
  UnixNanos ts_event;
};

class Record {
 public:
  explicit Record(RecordHeader* record) : record_{record} {}

  const RecordHeader& header() const;

  template <typename T>
  bool holds() const {
    return record_->rtype == T::kTypeId;
  }

  template <typename T>
  const T& get() const {
    return *reinterpret_cast<const T*>(record_);
  }
  template <typename T>
  T& get() {
    return *reinterpret_cast<T*>(record_);
  }

  std::size_t size() const;
  static std::size_t SizeOfType(std::uint8_t rtype);
  static std::uint8_t TypeIdFromSchema(Schema schema);

 private:
  RecordHeader* record_;
};

// Market-by-order (MBO) tick message.
struct TickMsg {
  static constexpr std::uint8_t kTypeId = 0xA0;

  RecordHeader hd;
  std::uint64_t order_id;
  std::int64_t price;
  std::uint32_t size;
  std::int8_t flags;
  std::uint8_t channel_id;
  char action;
  char side;
  UnixNanos ts_recv;
  TimeDeltaNanos ts_in_delta;
  std::uint32_t sequence;
};

static_assert(sizeof(TickMsg) == 56, "TickMsg size must match C");

struct BidAskPair {
  std::int64_t bid_px;
  std::int64_t ask_px;
  std::uint32_t bid_sz;
  std::uint32_t ask_sz;
  std::uint32_t bid_ct;
  std::uint32_t ask_ct;
};

static_assert(sizeof(BidAskPair) == 32, "BidAskPair size must match C");

namespace detail {
template <std::size_t N>
struct MbpMsg {
  static constexpr std::uint8_t kTypeId = N;

  RecordHeader hd;
  std::int64_t price;
  std::uint32_t size;
  char action;
  char side;
  std::int8_t flags;
  // Depth of the actual book change.
  std::uint8_t depth;
  UnixNanos ts_recv;
  TimeDeltaNanos ts_in_delta;
  std::uint32_t sequence;
  std::array<BidAskPair, N> booklevel;
};
}  // namespace detail

struct TradeMsg {
  static constexpr std::uint8_t kTypeId = 0;

  RecordHeader hd;
  std::int64_t price;
  std::uint32_t size;
  char action;
  char side;
  std::int8_t flags;
  // Depth of the actual book change.
  std::uint8_t depth;
  UnixNanos ts_recv;
  TimeDeltaNanos ts_in_delta;
  std::uint32_t sequence;
  // 0-sized types don't exist in C++ so booklevel is omitted
};

using Mbp1Msg = detail::MbpMsg<1>;
using TbboMsg = Mbp1Msg;
using Mbp10Msg = detail::MbpMsg<10>;

static_assert(sizeof(TradeMsg) == 48, "TradeMsg size must match C");
static_assert(sizeof(Mbp1Msg) == sizeof(TradeMsg) + sizeof(BidAskPair),
              "Mbp1Msg size must match C");

struct OhlcvMsg {
  static constexpr std::uint8_t kTypeId = 0x11;

  RecordHeader hd;
  std::int64_t open;
  std::int64_t high;
  std::int64_t low;
  std::int64_t close;
  std::uint64_t volume;
};

static_assert(sizeof(OhlcvMsg) == 56, "OhlcvMsg size must match C");

inline bool operator==(const RecordHeader& lhs, const RecordHeader& rhs) {
  return lhs.length == rhs.length && lhs.rtype == rhs.rtype &&
         lhs.publisher_id == rhs.publisher_id &&
         lhs.product_id == rhs.product_id && lhs.ts_event == rhs.ts_event;
}

inline bool operator==(const TickMsg& lhs, const TickMsg& rhs) {
  return lhs.hd == rhs.hd && lhs.order_id == rhs.order_id &&
         lhs.price == rhs.price && lhs.size == rhs.size &&
         lhs.flags == rhs.flags && lhs.channel_id == rhs.channel_id &&
         lhs.action == rhs.action && lhs.side == rhs.side &&
         lhs.ts_recv == rhs.ts_recv && lhs.ts_in_delta == rhs.ts_in_delta &&
         lhs.sequence == rhs.sequence;
}

inline bool operator==(const BidAskPair& lhs, const BidAskPair& rhs) {
  return lhs.bid_px == rhs.bid_px && lhs.ask_px == rhs.ask_px &&
         lhs.bid_sz == rhs.bid_sz && lhs.ask_sz == rhs.ask_sz &&
         lhs.bid_ct == rhs.bid_ct && lhs.ask_ct == rhs.ask_ct;
}

namespace detail {
template <std::size_t N>
bool operator==(const MbpMsg<N>& lhs, const MbpMsg<N>& rhs) {
  return lhs.hd == rhs.hd && lhs.price == rhs.price && lhs.size == rhs.size &&
         lhs.action == rhs.action && lhs.side == rhs.side &&
         lhs.flags == rhs.flags && lhs.depth == rhs.depth &&
         lhs.ts_recv == rhs.ts_recv && lhs.ts_in_delta == rhs.ts_in_delta &&
         lhs.sequence == rhs.sequence && lhs.booklevel == rhs.booklevel;
}
}  // namespace detail

inline bool operator==(const TradeMsg& lhs, const TradeMsg& rhs) {
  return lhs.hd == rhs.hd && lhs.price == rhs.price && lhs.size == rhs.size &&
         lhs.action == rhs.action && lhs.side == rhs.side &&
         lhs.flags == rhs.flags && lhs.depth == rhs.depth &&
         lhs.ts_recv == rhs.ts_recv && lhs.ts_in_delta == rhs.ts_in_delta &&
         lhs.sequence == rhs.sequence;
}

inline bool operator==(const OhlcvMsg& lhs, const OhlcvMsg& rhs) {
  return lhs.hd == rhs.hd && lhs.open == rhs.open && lhs.high == rhs.high &&
         lhs.low == rhs.low && lhs.close == rhs.close &&
         lhs.volume == rhs.volume;
}
}  // namespace databento
