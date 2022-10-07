#pragma once

#include <array>
#include <chrono>  // nanoseconds
#include <cstddef>
#include <cstdint>

#include "databento/datetime.hpp"  // EpochNanos
#include "databento/enums.hpp"

namespace databento {
/// Common data for all Databento Records.
struct RecordHeader {
  /// The length of the message in 32-bit words.
  std::uint8_t length;
  /// The record type.
  std::uint8_t rtype;
  /// The publisher ID assigned by Databento.
  std::uint16_t publisher_id;
  /// The product ID assigned by the venue.
  std::uint32_t product_id;
  /// The exchange timestamp in UTC UNIX epoch nanoseconds.
  EpochNanos ts_event;
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

/// Market-by-order (MBO) tick message.
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
  EpochNanos ts_recv;
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
  /// Depth of the actual book change.
  std::uint8_t depth;
  EpochNanos ts_recv;
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
  /// Depth of the actual book change.
  std::uint8_t depth;
  EpochNanos ts_recv;
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
}  // namespace databento
