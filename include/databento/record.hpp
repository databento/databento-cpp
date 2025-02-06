#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>  // strncmp
#include <string>
#include <tuple>  // tie
#include <type_traits>

#include "databento/constants.hpp"  // kSymbolCstrLen
#include "databento/datetime.hpp"   // UnixNanos
#include "databento/enums.hpp"
#include "databento/exceptions.hpp"  // InvalidArgumentError
#include "databento/flag_set.hpp"    // FlagSet
#include "databento/publishers.hpp"  // Publisher

namespace databento {
// Forward declare
namespace v3 {
struct InstrumentDefMsg;
}

// Common data for all Databento Records.
struct RecordHeader {
  static constexpr std::size_t kLengthMultiplier =
      kRecordHeaderLengthMultiplier;

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
  enum Publisher Publisher() const {
    return static_cast<enum Publisher>(publisher_id);
  }
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

// Market-by-order (MBO) message.
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
static_assert(alignof(MboMsg) == 8, "Must have 8-byte alignment");

struct BidAskPair {
  std::int64_t bid_px;
  std::int64_t ask_px;
  std::uint32_t bid_sz;
  std::uint32_t ask_sz;
  std::uint32_t bid_ct;
  std::uint32_t ask_ct;
};
static_assert(sizeof(BidAskPair) == 32, "BidAskPair size must match Rust");
static_assert(alignof(BidAskPair) == 8, "Must have 8-byte alignment");

struct ConsolidatedBidAskPair {
  std::int64_t bid_px;
  std::int64_t ask_px;
  std::uint32_t bid_sz;
  std::uint32_t ask_sz;
  std::uint16_t bid_pb;
  std::array<char, 2> reserved1;
  std::uint16_t ask_pb;
  std::array<char, 2> reserved2;
};
static_assert(sizeof(ConsolidatedBidAskPair) == 32,
              "ConsolidatedBidAskPair size must match Rust");
static_assert(alignof(ConsolidatedBidAskPair) == 8,
              "Must have 8-byte alignment");

struct TradeMsg {
  static bool HasRType(RType rtype) { return rtype == RType::Mbp0; }

  UnixNanos IndexTs() const { return ts_recv; }

  RecordHeader hd;
  std::int64_t price;
  std::uint32_t size;
  Action action;
  Side side;
  FlagSet flags;
  // Depth of the actual book change.
  std::uint8_t depth;
  UnixNanos ts_recv;
  TimeDeltaNanos ts_in_delta;
  std::uint32_t sequence;
};
static_assert(sizeof(TradeMsg) == 48, "TradeMsg size must match Rust");
static_assert(alignof(TradeMsg) == 8, "Must have 8-byte alignment");

struct Mbp1Msg {
  static bool HasRType(RType rtype) {
    switch (rtype) {
      case RType::Mbp1:
        return true;
      default:
        return false;
    };
  }

  UnixNanos IndexTs() const { return ts_recv; }

  RecordHeader hd;
  std::int64_t price;
  std::uint32_t size;
  Action action;
  Side side;
  FlagSet flags;
  // Depth of the actual book change.
  std::uint8_t depth;
  UnixNanos ts_recv;
  TimeDeltaNanos ts_in_delta;
  std::uint32_t sequence;
  std::array<BidAskPair, 1> levels;
};
using TbboMsg = Mbp1Msg;
static_assert(alignof(Mbp1Msg) == 8, "Must have 8-byte alignment");
static_assert(sizeof(Mbp1Msg) == sizeof(TradeMsg) + sizeof(BidAskPair),
              "Mbp1Msg size must match Rust");

struct Mbp10Msg {
  static bool HasRType(RType rtype) { return rtype == rtype::Mbp10; }

  UnixNanos IndexTs() const { return ts_recv; }

  RecordHeader hd;
  std::int64_t price;
  std::uint32_t size;
  Action action;
  Side side;
  FlagSet flags;
  // Depth of the actual book change.
  std::uint8_t depth;
  UnixNanos ts_recv;
  TimeDeltaNanos ts_in_delta;
  std::uint32_t sequence;
  std::array<BidAskPair, 10> levels;
};
static_assert(alignof(Mbp10Msg) == 8, "Must have 8-byte alignment");
static_assert(sizeof(Mbp10Msg) == sizeof(TradeMsg) + sizeof(BidAskPair) * 10,
              "Mbp10Msg size must match Rust");

struct BboMsg {
  static bool HasRType(RType rtype) {
    switch (rtype) {
      case RType::Bbo1S:  // fallthrough
      case RType::Bbo1M:
        return true;
      default:
        return false;
    };
  }

  UnixNanos IndexTs() const { return ts_recv; }

  RecordHeader hd;
  std::int64_t price;
  std::uint32_t size;
  char reserved1;
  Side side;
  FlagSet flags;
  char reserved2;
  UnixNanos ts_recv;
  std::array<char, 4> reserved3;
  std::uint32_t sequence;
  std::array<BidAskPair, 1> levels;
};
using Bbo1SMsg = BboMsg;
using Bbo1MMsg = BboMsg;
static_assert(alignof(BboMsg) == 8, "Must have 8-byte alignment");
static_assert(sizeof(BboMsg) == sizeof(Mbp1Msg), "BboMsg size must match Rust");

struct Cmbp1Msg {
  static bool HasRType(RType rtype) {
    switch (rtype) {
      case RType::Cmbp1:  // fallthrough
      case RType::Tcbbo:
        return true;
      default:
        return false;
    };
  }

  UnixNanos IndexTs() const { return ts_recv; }

  RecordHeader hd;
  std::int64_t price;
  std::uint32_t size;
  char action;
  Side side;
  FlagSet flags;
  char reserved1;
  UnixNanos ts_recv;
  TimeDeltaNanos ts_in_delta;
  std::array<char, 4> reserved2;
  std::array<ConsolidatedBidAskPair, 1> levels;
};
using TcbboMsg = Cmbp1Msg;
static_assert(alignof(Cmbp1Msg) == 8, "Must have 8-byte alignment");
static_assert(sizeof(Cmbp1Msg) ==
                  sizeof(TradeMsg) + sizeof(ConsolidatedBidAskPair),
              "Cmbp1Msg size must match Rust");

struct CbboMsg {
  static bool HasRType(RType rtype) {
    switch (rtype) {
      case RType::Cbbo1S:  // fallthrough
      case RType::Cbbo1M:  // fallthrough
        return true;
      default:
        return false;
    };
  }
  static_assert(alignof(Cmbp1Msg) == 8, "Must have 8-byte alignment");
  static_assert(sizeof(Cmbp1Msg) ==
                    sizeof(TradeMsg) + sizeof(ConsolidatedBidAskPair),
                "Cmbp1Msg size must match Rust");

  UnixNanos IndexTs() const { return ts_recv; }

  RecordHeader hd;
  std::int64_t price;
  std::uint32_t size;
  char reserved1;
  Side side;
  FlagSet flags;
  char reserved2;
  UnixNanos ts_recv;
  std::array<char, 4> reserved3;
  std::array<char, 4> reserved4;
  std::array<ConsolidatedBidAskPair, 1> levels;
};
using Cbbo1SMsg = CbboMsg;
using Cbbo1MMsg = CbboMsg;
static_assert(alignof(CbboMsg) == 8, "Must have 8-byte alignment");
static_assert(sizeof(CbboMsg) ==
                  sizeof(TradeMsg) + sizeof(ConsolidatedBidAskPair),
              "CbboMsg size must match Rust");

// Aggregate of open, high, low, and close prices with volume.
struct OhlcvMsg {
  static bool HasRType(RType rtype) {
    switch (rtype) {
      case RType::OhlcvDeprecated:  // fallthrough
      case RType::Ohlcv1S:          // fallthrough
      case RType::Ohlcv1M:          // fallthrough
      case RType::Ohlcv1H:          // fallthrough
      case RType::Ohlcv1D:
        return true;
      default:
        return false;
    }
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
static_assert(alignof(OhlcvMsg) == 8, "Must have 8-byte alignment");

// A trading status update message.
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
  std::array<char, 7> reserved;
};
static_assert(sizeof(StatusMsg) == 40, "StatusMsg size must match Rust");
static_assert(alignof(StatusMsg) == 8, "Must have 8-byte alignment");

// Instrument definition.
struct InstrumentDefMsg {
  static bool HasRType(RType rtype) { return rtype == RType::InstrumentDef; }

  v3::InstrumentDefMsg ToV3() const;
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
  const char* StrikePriceCurrency() const {
    return strike_price_currency.data();
  }

  RecordHeader hd;
  UnixNanos ts_recv;
  std::int64_t min_price_increment;
  std::int64_t display_factor;
  UnixNanos expiration;
  UnixNanos activation;
  std::int64_t high_limit_price;
  std::int64_t low_limit_price;
  std::int64_t max_price_variation;
  std::int64_t trading_reference_price;
  std::int64_t unit_of_measure_qty;
  std::int64_t min_price_increment_amount;
  std::int64_t price_ratio;
  std::int64_t strike_price;
  std::int32_t inst_attrib_value;
  std::uint32_t underlying_id;
  std::uint32_t raw_instrument_id;
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
  std::uint16_t trading_reference_date;
  std::int16_t appl_id;
  std::uint16_t maturity_year;
  std::uint16_t decay_start_date;
  std::uint16_t channel_id;
  std::array<char, 4> currency;
  std::array<char, 4> settl_currency;
  std::array<char, 6> secsubtype;
  std::array<char, kSymbolCstrLen> raw_symbol;
  std::array<char, 21> group;
  std::array<char, 5> exchange;
  std::array<char, 7> asset;
  std::array<char, 7> cfi;
  std::array<char, 7> security_type;
  std::array<char, 31> unit_of_measure;
  std::array<char, 21> underlying;
  std::array<char, 4> strike_price_currency;
  InstrumentClass instrument_class;
  MatchAlgorithm match_algorithm;
  std::uint8_t md_security_trading_status;
  std::uint8_t main_fraction;
  std::uint8_t price_display_format;
  std::uint8_t settl_price_type;
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
  std::array<char, 10> reserved;
};
static_assert(sizeof(InstrumentDefMsg) == 400,
              "InstrumentDefMsg size must match Rust");
static_assert(alignof(InstrumentDefMsg) == 8, "Must have 8-byte alignment");

// An order imbalance message.
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
  // padding for alignment
  std::array<char, 1> reserved;
};
static_assert(sizeof(ImbalanceMsg) == 112, "ImbalanceMsg size must match Rust");
static_assert(alignof(ImbalanceMsg) == 8, "Must have 8-byte alignment");

/// A statistics message. A catchall for various data disseminated by
/// publishers. The `stat_type` indicates the statistic contained in the
/// message.
struct StatMsg {
  static bool HasRType(RType rtype) { return rtype == RType::Statistics; }

  UnixNanos IndexTs() const { return ts_recv; }

  RecordHeader hd;
  UnixNanos ts_recv;
  UnixNanos ts_ref;
  std::int64_t price;
  std::int32_t quantity;
  std::uint32_t sequence;
  TimeDeltaNanos ts_in_delta;
  StatType stat_type;
  std::uint16_t channel_id;
  StatUpdateAction update_action;
  std::uint8_t stat_flags;
  std::array<char, 6> reserved;
};
static_assert(sizeof(StatMsg) == 64, "StatMsg size must match Rust");
static_assert(alignof(StatMsg) == 8, "Must have 8-byte alignment");

// An error message from the Live Subscription Gateway (LSG). This will never
// be present in historical data.
struct ErrorMsg {
  static bool HasRType(RType rtype) { return rtype == RType::Error; }

  UnixNanos IndexTs() const { return hd.ts_event; }
  const char* Err() const { return err.data(); }

  RecordHeader hd;
  std::array<char, 302> err;
  std::uint8_t code;
  std::uint8_t is_last;
};
static_assert(sizeof(ErrorMsg) == 320, "ErrorMsg size must match Rust");
static_assert(alignof(ErrorMsg) == 8, "Must have 8-byte alignment");

/// A symbol mapping message.
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
static_assert(sizeof(SymbolMappingMsg) == 176,
              "SymbolMappingMsg size must match Rust");
static_assert(alignof(SymbolMappingMsg) == 8, "Must have 8-byte alignment");

struct SystemMsg {
  static bool HasRType(RType rtype) { return rtype == RType::System; }

  UnixNanos IndexTs() const { return hd.ts_event; }
  const char* Msg() const { return msg.data(); }
  bool IsHeartbeat() const {
    return std::strncmp(msg.data(), "Heartbeat", 9) == 0;
  }

  RecordHeader hd;
  std::array<char, 303> msg;
  std::uint8_t code;
};
static_assert(sizeof(SystemMsg) == 320, "SystemMsg size must match Rust");
static_assert(alignof(SystemMsg) == 8, "Must have 8-byte alignment");

inline bool operator==(const RecordHeader& lhs, const RecordHeader& rhs) {
  return lhs.length == rhs.length && lhs.rtype == rhs.rtype &&
         lhs.publisher_id == rhs.publisher_id &&
         lhs.instrument_id == rhs.instrument_id && lhs.ts_event == rhs.ts_event;
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

inline bool operator==(const Mbp1Msg& lhs, const Mbp1Msg& rhs) {
  return lhs.hd == rhs.hd && lhs.price == rhs.price && lhs.size == rhs.size &&
         lhs.action == rhs.action && lhs.side == rhs.side &&
         lhs.flags == rhs.flags && lhs.depth == rhs.depth &&
         lhs.ts_recv == rhs.ts_recv && lhs.ts_in_delta == rhs.ts_in_delta &&
         lhs.sequence == rhs.sequence && lhs.levels == rhs.levels;
}
inline bool operator!=(const Mbp1Msg& lhs, const Mbp1Msg& rhs) {
  return !(lhs == rhs);
}

inline bool operator==(const Mbp10Msg& lhs, const Mbp10Msg& rhs) {
  return lhs.hd == rhs.hd && lhs.price == rhs.price && lhs.size == rhs.size &&
         lhs.action == rhs.action && lhs.side == rhs.side &&
         lhs.flags == rhs.flags && lhs.depth == rhs.depth &&
         lhs.ts_recv == rhs.ts_recv && lhs.ts_in_delta == rhs.ts_in_delta &&
         lhs.sequence == rhs.sequence && lhs.levels == rhs.levels;
}
inline bool operator!=(const Mbp10Msg& lhs, const Mbp10Msg& rhs) {
  return !(lhs == rhs);
}

inline bool operator==(const BboMsg& lhs, const BboMsg& rhs) {
  return std::tie(lhs.hd, lhs.price, lhs.size, lhs.side, lhs.flags, lhs.ts_recv,
                  lhs.sequence, lhs.levels) ==
         std::tie(rhs.hd, rhs.price, rhs.size, rhs.side, rhs.flags, rhs.ts_recv,
                  rhs.sequence, rhs.levels);
}
inline bool operator!=(const BboMsg& lhs, const BboMsg& rhs) {
  return !(lhs == rhs);
}

inline bool operator==(const Cmbp1Msg& lhs, const Cmbp1Msg& rhs) {
  return lhs.hd == rhs.hd && lhs.price == rhs.price && lhs.size == rhs.size &&
         lhs.action == rhs.action && lhs.side == rhs.side &&
         lhs.flags == rhs.flags && lhs.ts_recv == rhs.ts_recv &&
         lhs.ts_in_delta == rhs.ts_in_delta && lhs.levels == rhs.levels;
}
inline bool operator!=(const Cmbp1Msg& lhs, const Cmbp1Msg& rhs) {
  return !(lhs == rhs);
}

inline bool operator==(const CbboMsg& lhs, const CbboMsg& rhs) {
  return lhs.hd == rhs.hd && lhs.price == rhs.price && lhs.size == rhs.size &&
         lhs.side == rhs.side && lhs.flags == rhs.flags &&
         lhs.ts_recv == rhs.ts_recv && lhs.levels == rhs.levels;
}
inline bool operator!=(const CbboMsg& lhs, const CbboMsg& rhs) {
  return !(lhs == rhs);
}

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

inline bool operator==(const StatusMsg& lhs, const StatusMsg& rhs) {
  return std::tie(lhs.hd, lhs.ts_recv, lhs.action, lhs.reason,
                  lhs.trading_event, lhs.is_trading, lhs.is_quoting,
                  lhs.is_short_sell_restricted) ==
         std::tie(rhs.hd, rhs.ts_recv, rhs.action, rhs.reason,
                  rhs.trading_event, rhs.is_trading, rhs.is_quoting,
                  rhs.is_short_sell_restricted);
}
inline bool operator!=(const StatusMsg& lhs, const StatusMsg& rhs) {
  return !(lhs == rhs);
}

bool operator==(const InstrumentDefMsg& lhs, const InstrumentDefMsg& rhs);
inline bool operator!=(const InstrumentDefMsg& lhs,
                       const InstrumentDefMsg& rhs) {
  return !(lhs == rhs);
}

bool operator==(const ImbalanceMsg& lhs, const ImbalanceMsg& rhs);
inline bool operator!=(const ImbalanceMsg& lhs, const ImbalanceMsg& rhs) {
  return !(lhs == rhs);
}

inline bool operator==(const StatMsg& lhs, const StatMsg& rhs) {
  return std::tie(lhs.hd, lhs.ts_recv, lhs.ts_ref, lhs.price, lhs.quantity,
                  lhs.sequence, lhs.ts_in_delta, lhs.stat_type, lhs.channel_id,
                  lhs.update_action, lhs.stat_flags) ==
         std::tie(rhs.hd, rhs.ts_recv, rhs.ts_ref, rhs.price, rhs.quantity,
                  rhs.sequence, rhs.ts_in_delta, rhs.stat_type, rhs.channel_id,
                  rhs.update_action, rhs.stat_flags);
}
inline bool operator!=(const StatMsg& lhs, const StatMsg& rhs) {
  return !(lhs == rhs);
}

inline bool operator==(const ErrorMsg& lhs, const ErrorMsg& rhs) {
  return lhs.hd == rhs.hd && lhs.err == rhs.err && lhs.code == rhs.code &&
         lhs.is_last == rhs.is_last;
}
inline bool operator!=(const ErrorMsg& lhs, const ErrorMsg& rhs) {
  return !(lhs == rhs);
}

inline bool operator==(const SystemMsg& lhs, const SystemMsg& rhs) {
  return lhs.hd == rhs.hd && lhs.msg == rhs.msg && lhs.code == rhs.code;
}
inline bool operator!=(const SystemMsg& lhs, const SystemMsg& rhs) {
  return !(lhs == rhs);
}

inline bool operator==(const SymbolMappingMsg& lhs,
                       const SymbolMappingMsg& rhs) {
  return lhs.hd == rhs.hd && lhs.stype_in == rhs.stype_in &&
         lhs.stype_in_symbol == rhs.stype_in_symbol &&
         lhs.stype_out == rhs.stype_out &&
         lhs.stype_out_symbol == rhs.stype_out_symbol &&
         lhs.start_ts == rhs.start_ts && lhs.end_ts == rhs.end_ts;
}
inline bool operator!=(const SymbolMappingMsg& lhs,
                       const SymbolMappingMsg& rhs) {
  return !(lhs == rhs);
}

std::string ToString(const RecordHeader& header);
std::ostream& operator<<(std::ostream& stream, const RecordHeader& header);
std::string ToString(const Record& header);
std::ostream& operator<<(std::ostream& stream, const Record& header);
std::string ToString(const MboMsg& mbo_msg);
std::ostream& operator<<(std::ostream& stream, const MboMsg& mbo_msg);
std::string ToString(const BidAskPair& ba_pair);
std::ostream& operator<<(std::ostream& stream, const BidAskPair& ba_pair);
std::string ToString(const ConsolidatedBidAskPair& ba_pair);
std::ostream& operator<<(std::ostream& stream,
                         const ConsolidatedBidAskPair& ba_pair);
std::string ToString(const Mbp1Msg& mbp_msg);
std::ostream& operator<<(std::ostream& stream, const Mbp1Msg& mbp_msg);
std::string ToString(const Mbp10Msg& mbp_msg);
std::ostream& operator<<(std::ostream& stream, const Mbp10Msg& mbp_msg);
std::string ToString(const BboMsg& bbo_msg);
std::ostream& operator<<(std::ostream& stream, const BboMsg& bbo_msg);
std::string ToString(const CbboMsg& cbbo_msg);
std::ostream& operator<<(std::ostream& stream, const CbboMsg& cbbo_msg);
std::string ToString(const TradeMsg& trade_msg);
std::ostream& operator<<(std::ostream& stream, const TradeMsg& trade_msg);
std::string ToString(const OhlcvMsg& ohlcv_msg);
std::ostream& operator<<(std::ostream& stream, const OhlcvMsg& ohlcv_msg);
std::string ToString(const StatusMsg& status_msg);
std::ostream& operator<<(std::ostream& stream, const StatusMsg& status_msg);
std::string ToString(const InstrumentDefMsg& instr_def_msg);
std::ostream& operator<<(std::ostream& stream,
                         const InstrumentDefMsg& instr_def_msg);
std::string ToString(const ImbalanceMsg& imbalance_msg);
std::ostream& operator<<(std::ostream& stream,
                         const ImbalanceMsg& imbalance_msg);
std::string ToString(const StatMsg& stat_msg);
std::ostream& operator<<(std::ostream& stream, const StatMsg& stat_msg);
std::string ToString(const ErrorMsg& err_msg);
std::ostream& operator<<(std::ostream& stream, const ErrorMsg& err_msg);
std::string ToString(const SystemMsg& system_msg);
std::ostream& operator<<(std::ostream& stream, const SystemMsg& system_msg);
std::string ToString(const SymbolMappingMsg& symbol_mapping_msg);
std::ostream& operator<<(std::ostream& stream,
                         const SymbolMappingMsg& symbol_mapping_msg);

// The length in bytes of the largest record type.
static constexpr std::size_t kMaxRecordLen = 520 + 8;
}  // namespace databento
