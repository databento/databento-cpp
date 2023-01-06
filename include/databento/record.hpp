#pragma once

#include <array>
#include <chrono>  // nanoseconds
#include <cstddef>
#include <cstdint>
#include <string>

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
  bool Holds() const {
    return record_->rtype == T::kTypeId;
  }

  template <typename T>
  const T& Get() const {
    return *reinterpret_cast<const T*>(record_);
  }
  template <typename T>
  T& Get() {
    return *reinterpret_cast<T*>(record_);
  }

  std::size_t Size() const;
  static std::size_t SizeOfType(std::uint8_t rtype);
  static std::uint8_t TypeIdFromSchema(Schema schema);

 private:
  RecordHeader* record_;
};

// Market-by-order (MBO) message.
struct MboMsg {
  static constexpr std::uint8_t kTypeId = 0xA0;

  RecordHeader hd;
  std::uint64_t order_id;
  std::int64_t price;
  std::uint32_t size;
  std::uint8_t flags;
  std::uint8_t channel_id;
  char action;
  char side;
  UnixNanos ts_recv;
  TimeDeltaNanos ts_in_delta;
  std::uint32_t sequence;
};

static_assert(sizeof(MboMsg) == 56, "MboMsg size must match C");

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
  std::uint8_t flags;
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
  std::uint8_t flags;
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

// Aggregate of open, high, low, and close prices with volume.
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

// Instrument definition.
struct InstrumentDefMsg {
  static constexpr std::uint8_t kTypeId = 0x13;

  RecordHeader hd;
  UnixNanos ts_recv;
  std::int64_t min_price_increment;
  std::int64_t display_factor;
  std::uint64_t expiration;
  std::uint64_t activation;
  std::int64_t high_limit_price;
  std::int64_t low_limit_price;
  std::int64_t max_price_variation;
  std::int64_t trading_reference_price;
  std::int64_t unit_of_measure_qty;
  std::int64_t min_price_increment_amount;
  std::int64_t price_ratio;
  std::int32_t inst_attrib_value;
  std::uint32_t underlying_id;
  std::int32_t cleared_volume;
  std::int32_t market_depth_implied;
  std::int32_t market_depth;
  std::uint32_t market_segment_id;
  std::uint32_t max_trade_vol;
  std::int32_t min_lot_size;
  std::int32_t min_lot_size_block;
  std::int32_t min_lot_size_round_lot;
  std::uint32_t min_trade_vol;
  std::int32_t open_interest_qty;
  std::int32_t contract_multiplier;
  std::int32_t decay_quantity;
  std::int32_t original_contract_size;
  std::uint32_t related_security_id;
  std::uint16_t trading_reference_date;
  std::int16_t appl_id;
  std::uint16_t maturity_year;
  std::uint16_t decay_start_date;
  std::uint16_t channel_id;
  std::array<char, 4> currency;
  std::array<char, 4> settl_currency;
  std::array<char, 6> secsubtype;
  std::array<char, 22> symbol;
  std::array<char, 21> group;
  std::array<char, 5> exchange;
  std::array<char, 7> asset;
  std::array<char, 7> cfi;
  std::array<char, 7> security_type;
  std::array<char, 31> unit_of_measure;
  std::array<char, 21> underlying;
  std::array<char, 21> related;
  char match_algorithm;
  std::uint8_t md_security_trading_status;
  std::uint8_t main_fraction;
  std::uint8_t price_display_format;
  std::uint8_t settl_price_type;
  std::uint8_t sub_fraction;
  std::uint8_t underlying_product;
  char security_update_action;
  std::uint8_t maturity_month;
  std::uint8_t maturity_day;
  std::uint8_t maturity_week;
  char user_defined_instrument;
  std::int8_t contract_multiplier_unit;
  std::int8_t flow_schedule_type;
  std::uint8_t tick_rule;
  // padding for alignment
  std::array<char, 3> dummy;
};

static_assert(sizeof(InstrumentDefMsg) == 360,
              "InstrumentDefMsg size must match C");

inline bool operator==(const RecordHeader& lhs, const RecordHeader& rhs) {
  return lhs.length == rhs.length && lhs.rtype == rhs.rtype &&
         lhs.publisher_id == rhs.publisher_id &&
         lhs.product_id == rhs.product_id && lhs.ts_event == rhs.ts_event;
}
inline bool operator!=(const RecordHeader& lhs, const RecordHeader& rhs) {
  return !(lhs == rhs);
}

inline bool operator==(const MboMsg& lhs, const MboMsg& rhs) {
  return lhs.hd == rhs.hd && lhs.order_id == rhs.order_id &&
         lhs.price == rhs.price && lhs.size == rhs.size &&
         lhs.flags == rhs.flags && lhs.channel_id == rhs.channel_id &&
         lhs.action == rhs.action && lhs.side == rhs.side &&
         lhs.ts_recv == rhs.ts_recv && lhs.ts_in_delta == rhs.ts_in_delta &&
         lhs.sequence == rhs.sequence;
}
inline bool operator!=(const MboMsg& lhs, const MboMsg& rhs) {
  return !(lhs == rhs);
}

inline bool operator==(const BidAskPair& lhs, const BidAskPair& rhs) {
  return lhs.bid_px == rhs.bid_px && lhs.ask_px == rhs.ask_px &&
         lhs.bid_sz == rhs.bid_sz && lhs.ask_sz == rhs.ask_sz &&
         lhs.bid_ct == rhs.bid_ct && lhs.ask_ct == rhs.ask_ct;
}
inline bool operator!=(const BidAskPair& lhs, const BidAskPair& rhs) {
  return !(lhs == rhs);
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
template <std::size_t N>
bool operator!=(const MbpMsg<N>& lhs, const MbpMsg<N>& rhs) {
  return !(lhs == rhs);
}

template <std::size_t N>
std::string ToString(const MbpMsg<N>& mbp_msg);
template <>
std::string ToString(const Mbp1Msg& mbp_msg);
template <>
std::string ToString(const Mbp10Msg& mbp_msg);

template <std::size_t N>
std::ostream& operator<<(std::ostream& stream, const MbpMsg<N>& mbp_msg);
template <>
std::ostream& operator<<(std::ostream& stream, const Mbp1Msg& mbp_msg);
template <>
std::ostream& operator<<(std::ostream& stream, const Mbp10Msg& mbp_msg);
}  // namespace detail

inline bool operator==(const TradeMsg& lhs, const TradeMsg& rhs) {
  return lhs.hd == rhs.hd && lhs.price == rhs.price && lhs.size == rhs.size &&
         lhs.action == rhs.action && lhs.side == rhs.side &&
         lhs.flags == rhs.flags && lhs.depth == rhs.depth &&
         lhs.ts_recv == rhs.ts_recv && lhs.ts_in_delta == rhs.ts_in_delta &&
         lhs.sequence == rhs.sequence;
}
inline bool operator!=(const TradeMsg& lhs, const TradeMsg& rhs) {
  return !(lhs == rhs);
}

inline bool operator==(const OhlcvMsg& lhs, const OhlcvMsg& rhs) {
  return lhs.hd == rhs.hd && lhs.open == rhs.open && lhs.high == rhs.high &&
         lhs.low == rhs.low && lhs.close == rhs.close &&
         lhs.volume == rhs.volume;
}
inline bool operator!=(const OhlcvMsg& lhs, const OhlcvMsg& rhs) {
  return !(lhs == rhs);
}

bool operator==(const InstrumentDefMsg& lhs, const InstrumentDefMsg& rhs);
inline bool operator!=(const InstrumentDefMsg& lhs,
                       const InstrumentDefMsg& rhs) {
  return !(lhs == rhs);
}

std::string ToString(const RecordHeader& header);
std::ostream& operator<<(std::ostream& stream, const RecordHeader& header);
std::string ToString(const MboMsg& mbo_msg);
std::ostream& operator<<(std::ostream& stream, const MboMsg& mbo_msg);
std::string ToString(const BidAskPair& ba_pair);
std::ostream& operator<<(std::ostream& stream, const BidAskPair& ba_pair);
std::string ToString(const TradeMsg& trade_msg);
std::ostream& operator<<(std::ostream& stream, const TradeMsg& trade_msg);
std::string ToString(const OhlcvMsg& ohlcv_msg);
std::ostream& operator<<(std::ostream& stream, const OhlcvMsg& ohlcv_msg);
std::string ToString(const InstrumentDefMsg& instr_def_msg);
std::ostream& operator<<(std::ostream& stream,
                         const InstrumentDefMsg& instr_def_msg);
}  // namespace databento
