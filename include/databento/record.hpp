#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>  // strncmp
#include <limits>
#include <string>
#include <type_traits>

#include "databento/constants.hpp"  // kSymbolCstrLen
#include "databento/datetime.hpp"   // UnixNanos
#include "databento/enums.hpp"
#include "databento/exceptions.hpp"  // InvalidArgumentError
#include "databento/flag_set.hpp"    // FlagSet
#include "databento/publishers.hpp"  // Publisher

namespace databento {
// Common data for all Databento Records.
struct RecordHeader {
  static constexpr std::size_t kLengthMultiplier = kRecordHeaderLengthMultiplier;

  // The length of the message in 32-bit words.
  std::uint8_t length;
  // The record type.
  RType rtype;
  // The publisher ID assigned by Databento, which denotes the dataset and
  // venue.
  std::uint16_t publisher_id;
  // The numeric ID assigned to the instrument.
  std::uint32_t instrument_id;
  // The exchange timestamp in UNIX epoch nanoseconds.
  UnixNanos ts_event;

  std::size_t Size() const;
  // Retrieve the publisher based on the publisher ID.
  enum Publisher Publisher() const { return static_cast<enum Publisher>(publisher_id); }
};

// Type trait helper for templated functions accepting DBN records.
template <typename, typename = std::void_t<>>
struct has_header : std::false_type {};
template <typename T>
struct has_header<T, std::void_t<decltype(std::declval<T>().hd)>>
    : std::is_same<decltype(std::declval<T>().hd), RecordHeader> {};

class Record {
 public:
  explicit Record(RecordHeader* record) : record_{record} {}

  const RecordHeader& Header() const { return *record_; }
  ::databento::RType RType() const { return record_->rtype; }
  enum Publisher Publisher() const { return Header().Publisher(); }

  template <typename T>
  bool Holds() const {
    return T::HasRType(record_->rtype);
  }

  template <typename T>
  const T& Get() const {
    if (const auto* r = GetIf<T>()) {
      return *r;
    }
    throw InvalidArgumentError{
        "Get", "T", std::string{"rtype mismatch, found "} + ToString(RType())};
  }
  template <typename T>
  T& Get() {
    if (auto* r = GetIf<T>()) {
      return *r;
    }
    throw InvalidArgumentError{
        "Get", "T", std::string{"rtype mismatch, found "} + ToString(RType())};
  }
  template <typename T>
  const T* GetIf() const {
    if (!Holds<T>()) {
      return nullptr;
    }
    return reinterpret_cast<const T*>(record_);
  }
  template <typename T>
  T* GetIf() {
    if (!Holds<T>()) {
      return nullptr;
    }
    return reinterpret_cast<T*>(record_);
  }

  std::size_t Size() const;
  static std::size_t SizeOfSchema(Schema schema);
  static ::databento::RType RTypeFromSchema(Schema schema);

 private:
  RecordHeader* record_;
};

// A market-by-order (MBO) tick message. The record of the MBO schema.
struct MboMsg {
  static bool HasRType(RType rtype) { return rtype == RType::Mbo; }

  UnixNanos IndexTs() const { return ts_recv; }

  RecordHeader hd;
  std::uint64_t order_id;
  std::int64_t price;
  std::uint32_t size;
  FlagSet flags;
  std::uint8_t channel_id;
  Action action;
  Side side;
  UnixNanos ts_recv;
  TimeDeltaNanos ts_in_delta;
  std::uint32_t sequence;
};
static_assert(sizeof(MboMsg) == 56, "MboMsg size must match Rust");
static_assert(alignof(MboMsg) == 8, "MboMsg must have 8-byte alignment");

// A price level.
struct BidAskPair {
  std::int64_t bid_px;
  std::int64_t ask_px;
  std::uint32_t bid_sz;
  std::uint32_t ask_sz;
  std::uint32_t bid_ct;
  std::uint32_t ask_ct;
};
static_assert(sizeof(BidAskPair) == 32, "BidAskPair size must match Rust");
static_assert(alignof(BidAskPair) == 8, "BidAskPair must have 8-byte alignment");

// A price level consolidated from multiple venues.
struct ConsolidatedBidAskPair {
  std::int64_t bid_px;
  std::int64_t ask_px;
  std::uint32_t bid_sz;
  std::uint32_t ask_sz;
  std::uint16_t bid_pb;
  std::array<std::byte, 2> _reserved1{};
  std::uint16_t ask_pb;
  std::array<std::byte, 2> _reserved2{};
};
static_assert(sizeof(ConsolidatedBidAskPair) == 32,
              "ConsolidatedBidAskPair size must match Rust");
static_assert(alignof(ConsolidatedBidAskPair) == 8,
              "ConsolidatedBidAskPair must have 8-byte alignment");

// Market-by-price implementation with a book depth of 0. Equivalent to MBP-0. The
// record of the Trades schema.
struct TradeMsg {
  static bool HasRType(RType rtype) { return rtype == RType::Mbp0; }

  UnixNanos IndexTs() const { return ts_recv; }

  RecordHeader hd;
  std::int64_t price;
  std::uint32_t size;
  Action action;
  Side side;
  FlagSet flags;
  std::uint8_t depth;
  UnixNanos ts_recv;
  TimeDeltaNanos ts_in_delta;
  std::uint32_t sequence;
};
static_assert(sizeof(TradeMsg) == 48, "TradeMsg size must match Rust");
static_assert(alignof(TradeMsg) == 8, "TradeMsg must have 8-byte alignment");

// Market-by-price implementation with a known book depth of 1. The record of the MBP-1
// schema.
struct Mbp1Msg {
  static bool HasRType(RType rtype) { return rtype == RType::Mbp1; }

  UnixNanos IndexTs() const { return ts_recv; }

  RecordHeader hd;
  std::int64_t price;
  std::uint32_t size;
  Action action;
  Side side;
  FlagSet flags;
  std::uint8_t depth;
  UnixNanos ts_recv;
  TimeDeltaNanos ts_in_delta;
  std::uint32_t sequence;
  std::array<BidAskPair, 1> levels;
};
static_assert(sizeof(Mbp1Msg) == 80, "Mbp1Msg size must match Rust");
static_assert(alignof(Mbp1Msg) == 8, "Mbp1Msg must have 8-byte alignment");

// Market-by-price implementation with a known book depth of 10. The record of the
// MBP-10
// schema.
struct Mbp10Msg {
  static bool HasRType(RType rtype) { return rtype == RType::Mbp10; }

  UnixNanos IndexTs() const { return ts_recv; }

  RecordHeader hd;
  std::int64_t price;
  std::uint32_t size;
  Action action;
  Side side;
  FlagSet flags;
  std::uint8_t depth;
  UnixNanos ts_recv;
  TimeDeltaNanos ts_in_delta;
  std::uint32_t sequence;
  std::array<BidAskPair, 10> levels;
};
static_assert(sizeof(Mbp10Msg) == 368, "Mbp10Msg size must match Rust");
static_assert(alignof(Mbp10Msg) == 8, "Mbp10Msg must have 8-byte alignment");

// Subsampled market by price with a known book depth of 1. The record of the BBO-1s and
// BBO-1m schemas.
struct BboMsg {
  static bool HasRType(RType rtype) {
    return rtype == RType::Bbo1S || rtype == RType::Bbo1M;
  }

  UnixNanos IndexTs() const { return ts_recv; }

  RecordHeader hd;
  std::int64_t price;
  std::uint32_t size;
  std::uint8_t _reserved1{};
  Side side;
  FlagSet flags;
  std::uint8_t _reserved2{};
  UnixNanos ts_recv;
  std::array<std::byte, 4> _reserved3{};
  std::uint32_t sequence;
  std::array<BidAskPair, 1> levels;
};
static_assert(sizeof(BboMsg) == 80, "BboMsg size must match Rust");
static_assert(alignof(BboMsg) == 8, "BboMsg must have 8-byte alignment");

// Consolidated market-by-price implementation with a known book depth of 1. The record
// of
// the CMBP-1 schema.
struct Cmbp1Msg {
  static bool HasRType(RType rtype) {
    return rtype == RType::Cmbp1 || rtype == RType::Tcbbo;
  }

  UnixNanos IndexTs() const { return ts_recv; }

  RecordHeader hd;
  std::int64_t price;
  std::uint32_t size;
  Action action;
  Side side;
  FlagSet flags;
  std::array<std::byte, 1> _reserved1{};
  UnixNanos ts_recv;
  TimeDeltaNanos ts_in_delta;
  std::array<std::byte, 4> _reserved2{};
  std::array<ConsolidatedBidAskPair, 1> levels;
};
static_assert(sizeof(Cmbp1Msg) == 80, "Cmbp1Msg size must match Rust");
static_assert(alignof(Cmbp1Msg) == 8, "Cmbp1Msg must have 8-byte alignment");

// Subsampled consolidated market by price with a known book depth of 1. The record of
// the CBBO-1s and CBBO-1m schemas.
struct CbboMsg {
  static bool HasRType(RType rtype) {
    return rtype == RType::Cbbo1S || rtype == RType::Cbbo1M;
  }

  UnixNanos IndexTs() const { return ts_recv; }

  RecordHeader hd;
  std::int64_t price;
  std::uint32_t size;
  std::uint8_t _reserved1{};
  Side side;
  FlagSet flags;
  std::uint8_t _reserved2{};
  UnixNanos ts_recv;
  std::array<std::byte, 8> _reserved3{};
  std::array<ConsolidatedBidAskPair, 1> levels;
};
static_assert(sizeof(CbboMsg) == 80, "CbboMsg size must match Rust");
static_assert(alignof(CbboMsg) == 8, "CbboMsg must have 8-byte alignment");

// The record of the TBBO schema.
using TbboMsg = Mbp1Msg;
// The record of the BBO-1s schema.
using Bbo1SMsg = BboMsg;
// The record of the BBO-1m schema.
using Bbo1MMsg = BboMsg;
// The record of the TCBBO schema.
using TcbboMsg = Cmbp1Msg;
// The record of the CBBO-1s schema.
using Cbbo1SMsg = CbboMsg;
// The record of the CBBO-1m schema.
using Cbbo1MMsg = CbboMsg;
// Open, high, low, close, and volume. The record of the following schemas:
// - OHLCV-1s
// - OHLCV-1m
// - OHLCV-1h
// - OHLCV-1d
// - OHLCV-eod
struct OhlcvMsg {
  static bool HasRType(RType rtype) {
    return rtype == RType::Ohlcv1S || rtype == RType::Ohlcv1M ||
           rtype == RType::Ohlcv1H || rtype == RType::Ohlcv1D ||
           rtype == RType::OhlcvEod || rtype == RType::OhlcvDeprecated;
  }

  UnixNanos IndexTs() const { return hd.ts_event; }

  RecordHeader hd;
  std::int64_t open;
  std::int64_t high;
  std::int64_t low;
  std::int64_t close;
  std::uint64_t volume;
};
static_assert(sizeof(OhlcvMsg) == 56, "OhlcvMsg size must match Rust");
static_assert(alignof(OhlcvMsg) == 8, "OhlcvMsg must have 8-byte alignment");

// A trading status update message. The record of the status schema.
struct StatusMsg {
  static bool HasRType(RType rtype) { return rtype == RType::Status; }

  UnixNanos IndexTs() const { return ts_recv; }

  RecordHeader hd;
  UnixNanos ts_recv;
  StatusAction action;
  StatusReason reason;
  TradingEvent trading_event;
  TriState is_trading;
  TriState is_quoting;
  TriState is_short_sell_restricted;
  std::array<std::byte, 7> _reserved{};
};
static_assert(sizeof(StatusMsg) == 40, "StatusMsg size must match Rust");
static_assert(alignof(StatusMsg) == 8, "StatusMsg must have 8-byte alignment");

// A definition of an instrument. The record of the definition schema.
struct InstrumentDefMsg {
  static bool HasRType(RType rtype) { return rtype == RType::InstrumentDef; }

  UnixNanos IndexTs() const { return ts_recv; }

  const char* Currency() const { return currency.data(); }
  const char* SettlCurrency() const { return settl_currency.data(); }
  const char* SecSubType() const { return secsubtype.data(); }
  const char* RawSymbol() const { return raw_symbol.data(); }
  const char* Group() const { return group.data(); }
  const char* Exchange() const { return exchange.data(); }
  const char* Asset() const { return asset.data(); }
  const char* Cfi() const { return cfi.data(); }
  const char* SecurityType() const { return security_type.data(); }
  const char* UnitOfMeasure() const { return unit_of_measure.data(); }
  const char* Underlying() const { return underlying.data(); }
  const char* StrikePriceCurrency() const { return strike_price_currency.data(); }
  const char* LegRawSymbol() const { return leg_raw_symbol.data(); }

  RecordHeader hd;
  UnixNanos ts_recv;
  std::int64_t min_price_increment;
  std::int64_t display_factor;
  UnixNanos expiration;
  UnixNanos activation;
  std::int64_t high_limit_price;
  std::int64_t low_limit_price;
  std::int64_t max_price_variation;
  std::int64_t unit_of_measure_qty;
  std::int64_t min_price_increment_amount;
  std::int64_t price_ratio;
  std::int64_t strike_price;
  std::uint64_t raw_instrument_id;
  std::int64_t leg_price;
  std::int64_t leg_delta;
  std::int32_t inst_attrib_value;
  std::uint32_t underlying_id;
  std::int32_t market_depth_implied;
  std::int32_t market_depth;
  std::uint32_t market_segment_id;
  std::uint32_t max_trade_vol;
  std::int32_t min_lot_size;
  std::int32_t min_lot_size_block;
  std::int32_t min_lot_size_round_lot;
  std::uint32_t min_trade_vol;
  std::int32_t contract_multiplier;
  std::int32_t decay_quantity;
  std::int32_t original_contract_size;
  std::uint32_t leg_instrument_id;
  std::int32_t leg_ratio_price_numerator;
  std::int32_t leg_ratio_price_denominator;
  std::int32_t leg_ratio_qty_numerator;
  std::int32_t leg_ratio_qty_denominator;
  std::uint32_t leg_underlying_id;
  std::int16_t appl_id;
  std::uint16_t maturity_year;
  std::uint16_t decay_start_date;
  std::uint16_t channel_id;
  std::uint16_t leg_count;
  std::uint16_t leg_index;
  std::array<char, 4> currency;
  std::array<char, 4> settl_currency;
  std::array<char, 6> secsubtype;
  std::array<char, kSymbolCstrLen> raw_symbol;
  std::array<char, 21> group;
  std::array<char, 5> exchange;
  std::array<char, kAssetCstrLen> asset;
  std::array<char, 7> cfi;
  std::array<char, 7> security_type;
  std::array<char, 31> unit_of_measure;
  std::array<char, 21> underlying;
  std::array<char, 4> strike_price_currency;
  std::array<char, kSymbolCstrLen> leg_raw_symbol;
  InstrumentClass instrument_class;
  MatchAlgorithm match_algorithm;
  std::uint8_t main_fraction;
  std::uint8_t price_display_format;
  std::uint8_t sub_fraction;
  std::uint8_t underlying_product;
  SecurityUpdateAction security_update_action;
  std::uint8_t maturity_month;
  std::uint8_t maturity_day;
  std::uint8_t maturity_week;
  UserDefinedInstrument user_defined_instrument;
  std::int8_t contract_multiplier_unit;
  std::int8_t flow_schedule_type;
  std::uint8_t tick_rule;
  InstrumentClass leg_instrument_class;
  Side leg_side;
  std::array<std::byte, 17> _reserved{};
};
static_assert(sizeof(InstrumentDefMsg) == 520, "InstrumentDefMsg size must match Rust");
static_assert(alignof(InstrumentDefMsg) == 8,
              "InstrumentDefMsg must have 8-byte alignment");

// An auction imbalance message.
struct ImbalanceMsg {
  static bool HasRType(RType rtype) { return rtype == RType::Imbalance; }

  UnixNanos IndexTs() const { return ts_recv; }

  RecordHeader hd;
  UnixNanos ts_recv;
  std::int64_t ref_price;
  UnixNanos auction_time;
  std::int64_t cont_book_clr_price;
  std::int64_t auct_interest_clr_price;
  std::int64_t ssr_filling_price;
  std::int64_t ind_match_price;
  std::int64_t upper_collar;
  std::int64_t lower_collar;
  std::uint32_t paired_qty;
  std::uint32_t total_imbalance_qty;
  std::uint32_t market_imbalance_qty;
  std::uint32_t unpaired_qty;
  char auction_type;
  Side side;
  std::uint8_t auction_status;
  std::uint8_t freeze_status;
  std::uint8_t num_extensions;
  Side unpaired_side;
  char significant_imbalance;
  std::array<std::byte, 1> _reserved{};
};
static_assert(sizeof(ImbalanceMsg) == 112, "ImbalanceMsg size must match Rust");
static_assert(alignof(ImbalanceMsg) == 8, "ImbalanceMsg must have 8-byte alignment");

// A statistics message. A catchall for various data disseminated by publishers. The
// `stat_type` indicates the statistic contained in the message.
struct StatMsg {
  static bool HasRType(RType rtype) { return rtype == RType::Statistics; }

  UnixNanos IndexTs() const { return ts_recv; }

  RecordHeader hd;
  UnixNanos ts_recv;
  UnixNanos ts_ref;
  std::int64_t price;
  std::int64_t quantity;
  std::uint32_t sequence;
  TimeDeltaNanos ts_in_delta;
  StatType stat_type;
  std::uint16_t channel_id;
  StatUpdateAction update_action;
  std::uint8_t stat_flags;
  std::array<std::byte, 18> _reserved{};
};
static_assert(sizeof(StatMsg) == 80, "StatMsg size must match Rust");
static_assert(alignof(StatMsg) == 8, "StatMsg must have 8-byte alignment");

// An error message from the Databento Live Subscription Gateway (LSG).
struct ErrorMsg {
  static bool HasRType(RType rtype) { return rtype == RType::Error; }

  UnixNanos IndexTs() const { return hd.ts_event; }

  const char* Err() const { return err.data(); }

  RecordHeader hd;
  std::array<char, 302> err;
  ErrorCode code;
  std::uint8_t is_last;
};
static_assert(sizeof(ErrorMsg) == 320, "ErrorMsg size must match Rust");
static_assert(alignof(ErrorMsg) == 8, "ErrorMsg must have 8-byte alignment");

// A symbol mapping message from the live API which maps a symbol from one `SType` to
// another.
struct SymbolMappingMsg {
  static bool HasRType(RType rtype) { return rtype == RType::SymbolMapping; }

  UnixNanos IndexTs() const { return hd.ts_event; }

  const char* STypeInSymbol() const { return stype_in_symbol.data(); }
  const char* STypeOutSymbol() const { return stype_out_symbol.data(); }

  RecordHeader hd;
  SType stype_in;
  std::array<char, kSymbolCstrLen> stype_in_symbol;
  SType stype_out;
  std::array<char, kSymbolCstrLen> stype_out_symbol;
  UnixNanos start_ts;
  UnixNanos end_ts;
};
static_assert(sizeof(SymbolMappingMsg) == 176, "SymbolMappingMsg size must match Rust");
static_assert(alignof(SymbolMappingMsg) == 8,
              "SymbolMappingMsg must have 8-byte alignment");

// A non-error message from the Databento Live Subscription Gateway (LSG). Also used
// for heartbeating.
struct SystemMsg {
  static bool HasRType(RType rtype) { return rtype == RType::System; }

  UnixNanos IndexTs() const { return hd.ts_event; }

  const char* Msg() const { return msg.data(); }
  bool IsHeartbeat() const {
    // Check if code is unset
    if (static_cast<std::uint8_t>(code) == std::numeric_limits<std::uint8_t>::max()) {
      return std::strncmp(msg.data(), "Heartbeat", 9) == 0;
    }
    return code == SystemCode::Heartbeat;
  }

  RecordHeader hd;
  std::array<char, 303> msg;
  SystemCode code;
};
static_assert(sizeof(SystemMsg) == 320, "SystemMsg size must match Rust");
static_assert(alignof(SystemMsg) == 8, "SystemMsg must have 8-byte alignment");

inline bool operator==(const RecordHeader& lhs, const RecordHeader& rhs) {
  return lhs.length == rhs.length && lhs.rtype == rhs.rtype &&
         lhs.publisher_id == rhs.publisher_id &&
         lhs.instrument_id == rhs.instrument_id && lhs.ts_event == rhs.ts_event;
}
inline bool operator!=(const RecordHeader& lhs, const RecordHeader& rhs) {
  return !(lhs == rhs);
}

inline bool operator==(const MboMsg& lhs, const MboMsg& rhs) {
  return lhs.hd == rhs.hd && lhs.order_id == rhs.order_id && lhs.price == rhs.price &&
         lhs.size == rhs.size && lhs.flags == rhs.flags &&
         lhs.channel_id == rhs.channel_id && lhs.action == rhs.action &&
         lhs.side == rhs.side && lhs.ts_recv == rhs.ts_recv &&
         lhs.ts_in_delta == rhs.ts_in_delta && lhs.sequence == rhs.sequence;
}
inline bool operator!=(const MboMsg& lhs, const MboMsg& rhs) { return !(lhs == rhs); }
inline bool operator==(const BidAskPair& lhs, const BidAskPair& rhs) {
  return lhs.bid_px == rhs.bid_px && lhs.ask_px == rhs.ask_px &&
         lhs.bid_sz == rhs.bid_sz && lhs.ask_sz == rhs.ask_sz &&
         lhs.bid_ct == rhs.bid_ct && lhs.ask_ct == rhs.ask_ct;
}
inline bool operator!=(const BidAskPair& lhs, const BidAskPair& rhs) {
  return !(lhs == rhs);
}
inline bool operator==(const ConsolidatedBidAskPair& lhs,
                       const ConsolidatedBidAskPair& rhs) {
  return lhs.bid_px == rhs.bid_px && lhs.ask_px == rhs.ask_px &&
         lhs.bid_sz == rhs.bid_sz && lhs.ask_sz == rhs.ask_sz &&
         lhs.bid_pb == rhs.bid_pb && lhs.ask_pb == rhs.ask_pb;
}
inline bool operator!=(const ConsolidatedBidAskPair& lhs,
                       const ConsolidatedBidAskPair& rhs) {
  return !(lhs == rhs);
}
inline bool operator==(const TradeMsg& lhs, const TradeMsg& rhs) {
  return lhs.hd == rhs.hd && lhs.price == rhs.price && lhs.size == rhs.size &&
         lhs.action == rhs.action && lhs.side == rhs.side && lhs.flags == rhs.flags &&
         lhs.depth == rhs.depth && lhs.ts_recv == rhs.ts_recv &&
         lhs.ts_in_delta == rhs.ts_in_delta && lhs.sequence == rhs.sequence;
}
inline bool operator!=(const TradeMsg& lhs, const TradeMsg& rhs) {
  return !(lhs == rhs);
}
inline bool operator==(const Mbp1Msg& lhs, const Mbp1Msg& rhs) {
  return lhs.hd == rhs.hd && lhs.price == rhs.price && lhs.size == rhs.size &&
         lhs.action == rhs.action && lhs.side == rhs.side && lhs.flags == rhs.flags &&
         lhs.depth == rhs.depth && lhs.ts_recv == rhs.ts_recv &&
         lhs.ts_in_delta == rhs.ts_in_delta && lhs.sequence == rhs.sequence &&
         lhs.levels == rhs.levels;
}
inline bool operator!=(const Mbp1Msg& lhs, const Mbp1Msg& rhs) { return !(lhs == rhs); }
inline bool operator==(const Mbp10Msg& lhs, const Mbp10Msg& rhs) {
  return lhs.hd == rhs.hd && lhs.price == rhs.price && lhs.size == rhs.size &&
         lhs.action == rhs.action && lhs.side == rhs.side && lhs.flags == rhs.flags &&
         lhs.depth == rhs.depth && lhs.ts_recv == rhs.ts_recv &&
         lhs.ts_in_delta == rhs.ts_in_delta && lhs.sequence == rhs.sequence &&
         lhs.levels == rhs.levels;
}
inline bool operator!=(const Mbp10Msg& lhs, const Mbp10Msg& rhs) {
  return !(lhs == rhs);
}
inline bool operator==(const BboMsg& lhs, const BboMsg& rhs) {
  return lhs.hd == rhs.hd && lhs.price == rhs.price && lhs.size == rhs.size &&
         lhs.side == rhs.side && lhs.flags == rhs.flags && lhs.ts_recv == rhs.ts_recv &&
         lhs.sequence == rhs.sequence && lhs.levels == rhs.levels;
}
inline bool operator!=(const BboMsg& lhs, const BboMsg& rhs) { return !(lhs == rhs); }
inline bool operator==(const Cmbp1Msg& lhs, const Cmbp1Msg& rhs) {
  return lhs.hd == rhs.hd && lhs.price == rhs.price && lhs.size == rhs.size &&
         lhs.action == rhs.action && lhs.side == rhs.side && lhs.flags == rhs.flags &&
         lhs.ts_recv == rhs.ts_recv && lhs.ts_in_delta == rhs.ts_in_delta &&
         lhs.levels == rhs.levels;
}
inline bool operator!=(const Cmbp1Msg& lhs, const Cmbp1Msg& rhs) {
  return !(lhs == rhs);
}
inline bool operator==(const CbboMsg& lhs, const CbboMsg& rhs) {
  return lhs.hd == rhs.hd && lhs.price == rhs.price && lhs.size == rhs.size &&
         lhs.side == rhs.side && lhs.flags == rhs.flags && lhs.ts_recv == rhs.ts_recv &&
         lhs.levels == rhs.levels;
}
inline bool operator!=(const CbboMsg& lhs, const CbboMsg& rhs) { return !(lhs == rhs); }
inline bool operator==(const OhlcvMsg& lhs, const OhlcvMsg& rhs) {
  return lhs.hd == rhs.hd && lhs.open == rhs.open && lhs.high == rhs.high &&
         lhs.low == rhs.low && lhs.close == rhs.close && lhs.volume == rhs.volume;
}
inline bool operator!=(const OhlcvMsg& lhs, const OhlcvMsg& rhs) {
  return !(lhs == rhs);
}
inline bool operator==(const StatusMsg& lhs, const StatusMsg& rhs) {
  return lhs.hd == rhs.hd && lhs.ts_recv == rhs.ts_recv && lhs.action == rhs.action &&
         lhs.reason == rhs.reason && lhs.trading_event == rhs.trading_event &&
         lhs.is_trading == rhs.is_trading && lhs.is_quoting == rhs.is_quoting &&
         lhs.is_short_sell_restricted == rhs.is_short_sell_restricted;
}
inline bool operator!=(const StatusMsg& lhs, const StatusMsg& rhs) {
  return !(lhs == rhs);
}

bool operator==(const InstrumentDefMsg& lhs, const InstrumentDefMsg& rhs);
inline bool operator!=(const InstrumentDefMsg& lhs, const InstrumentDefMsg& rhs) {
  return !(lhs == rhs);
}

bool operator==(const ImbalanceMsg& lhs, const ImbalanceMsg& rhs);
inline bool operator!=(const ImbalanceMsg& lhs, const ImbalanceMsg& rhs) {
  return !(lhs == rhs);
}
inline bool operator==(const StatMsg& lhs, const StatMsg& rhs) {
  return lhs.hd == rhs.hd && lhs.ts_recv == rhs.ts_recv && lhs.ts_ref == rhs.ts_ref &&
         lhs.price == rhs.price && lhs.quantity == rhs.quantity &&
         lhs.sequence == rhs.sequence && lhs.ts_in_delta == rhs.ts_in_delta &&
         lhs.stat_type == rhs.stat_type && lhs.channel_id == rhs.channel_id &&
         lhs.update_action == rhs.update_action && lhs.stat_flags == rhs.stat_flags;
}
inline bool operator!=(const StatMsg& lhs, const StatMsg& rhs) { return !(lhs == rhs); }
inline bool operator==(const ErrorMsg& lhs, const ErrorMsg& rhs) {
  return lhs.hd == rhs.hd && lhs.err == rhs.err && lhs.code == rhs.code &&
         lhs.is_last == rhs.is_last;
}
inline bool operator!=(const ErrorMsg& lhs, const ErrorMsg& rhs) {
  return !(lhs == rhs);
}
inline bool operator==(const SymbolMappingMsg& lhs, const SymbolMappingMsg& rhs) {
  return lhs.hd == rhs.hd && lhs.stype_in == rhs.stype_in &&
         lhs.stype_in_symbol == rhs.stype_in_symbol && lhs.stype_out == rhs.stype_out &&
         lhs.stype_out_symbol == rhs.stype_out_symbol && lhs.start_ts == rhs.start_ts &&
         lhs.end_ts == rhs.end_ts;
}
inline bool operator!=(const SymbolMappingMsg& lhs, const SymbolMappingMsg& rhs) {
  return !(lhs == rhs);
}
inline bool operator==(const SystemMsg& lhs, const SystemMsg& rhs) {
  return lhs.hd == rhs.hd && lhs.msg == rhs.msg && lhs.code == rhs.code;
}
inline bool operator!=(const SystemMsg& lhs, const SystemMsg& rhs) {
  return !(lhs == rhs);
}

std::string ToString(const RecordHeader& header);
std::ostream& operator<<(std::ostream& stream, const RecordHeader& header);
std::string ToString(const Record& header);
std::ostream& operator<<(std::ostream& stream, const Record& header);

std::string ToString(const MboMsg& mbo_msg);
std::ostream& operator<<(std::ostream& stream, const MboMsg& mbo_msg);
std::string ToString(const BidAskPair& bid_ask_pair);
std::ostream& operator<<(std::ostream& stream, const BidAskPair& bid_ask_pair);
std::string ToString(const ConsolidatedBidAskPair& consolidated_bid_ask_pair);
std::ostream& operator<<(std::ostream& stream,
                         const ConsolidatedBidAskPair& consolidated_bid_ask_pair);
std::string ToString(const TradeMsg& trade_msg);
std::ostream& operator<<(std::ostream& stream, const TradeMsg& trade_msg);
std::string ToString(const Mbp1Msg& mbp1_msg);
std::ostream& operator<<(std::ostream& stream, const Mbp1Msg& mbp1_msg);
std::string ToString(const Mbp10Msg& mbp10_msg);
std::ostream& operator<<(std::ostream& stream, const Mbp10Msg& mbp10_msg);
std::string ToString(const BboMsg& bbo_msg);
std::ostream& operator<<(std::ostream& stream, const BboMsg& bbo_msg);
std::string ToString(const Cmbp1Msg& cmbp1_msg);
std::ostream& operator<<(std::ostream& stream, const Cmbp1Msg& cmbp1_msg);
std::string ToString(const CbboMsg& cbbo_msg);
std::ostream& operator<<(std::ostream& stream, const CbboMsg& cbbo_msg);
std::string ToString(const OhlcvMsg& ohlcv_msg);
std::ostream& operator<<(std::ostream& stream, const OhlcvMsg& ohlcv_msg);
std::string ToString(const StatusMsg& status_msg);
std::ostream& operator<<(std::ostream& stream, const StatusMsg& status_msg);
std::string ToString(const InstrumentDefMsg& instrument_def_msg);
std::ostream& operator<<(std::ostream& stream,
                         const InstrumentDefMsg& instrument_def_msg);
std::string ToString(const ImbalanceMsg& imbalance_msg);
std::ostream& operator<<(std::ostream& stream, const ImbalanceMsg& imbalance_msg);
std::string ToString(const StatMsg& stat_msg);
std::ostream& operator<<(std::ostream& stream, const StatMsg& stat_msg);
std::string ToString(const ErrorMsg& error_msg);
std::ostream& operator<<(std::ostream& stream, const ErrorMsg& error_msg);
std::string ToString(const SymbolMappingMsg& symbol_mapping_msg);
std::ostream& operator<<(std::ostream& stream,
                         const SymbolMappingMsg& symbol_mapping_msg);
std::string ToString(const SystemMsg& system_msg);
std::ostream& operator<<(std::ostream& stream, const SystemMsg& system_msg);

// The length in bytes of the largest record type.
static constexpr std::size_t kMaxRecordLen = 520 + 8;
static_assert(kMaxRecordLen == sizeof(InstrumentDefMsg) + sizeof(UnixNanos),
              "v3 definition with ts_out should be the largest record");
}  // namespace databento
