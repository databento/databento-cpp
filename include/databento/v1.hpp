#pragma once

#include "databento/constants.hpp"  // kSymbolCstrLen
#include "databento/datetime.hpp"   // UnixNanos
#include "databento/enums.hpp"
#include "databento/record.hpp"
#include "databento/v2.hpp"

namespace databento {
// Forward declare
namespace v3 {
struct InstrumentDefMsg;
}

namespace v1 {
static constexpr std::size_t kSymbolCstrLen = 22;
static constexpr std::size_t kAssetCstrLen = databento::kAssetCstrLen;

using MboMsg = databento::MboMsg;
using TradeMsg = databento::TradeMsg;
using Mbp1Msg = databento::Mbp1Msg;
using TbboMsg = databento::TbboMsg;
using Mbp10Msg = databento::Mbp10Msg;
using BboMsg = databento::BboMsg;
using Bbo1SMsg = databento::Bbo1SMsg;
using Bbo1MMsg = databento::Bbo1MMsg;
using Cmbp1Msg = databento::Cmbp1Msg;
using TcbboMsg = databento::TcbboMsg;
using CbboMsg = databento::CbboMsg;
using Cbbo1SMsg = databento::Cbbo1SMsg;
using Cbbo1MMsg = databento::Cbbo1MMsg;
using OhlcvMsg = databento::OhlcvMsg;
using StatusMsg = databento::StatusMsg;
using ImbalanceMsg = databento::ImbalanceMsg;
using StatMsg = databento::StatMsg;

struct InstrumentDefMsg {
  static bool HasRType(RType rtype) { return rtype == RType::InstrumentDef; }

  v2::InstrumentDefMsg ToV2() const;
  v3::InstrumentDefMsg ToV3() const;
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
  std::array<char, 4> _reserved2;
  std::int32_t contract_multiplier;
  std::int32_t decay_quantity;
  std::int32_t original_contract_size;
  std::array<char, 4> _reserved3;
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
  std::array<char, kAssetCstrLen> asset;
  std::array<char, 7> cfi;
  std::array<char, 7> security_type;
  std::array<char, 31> unit_of_measure;
  std::array<char, 21> underlying;
  std::array<char, 4> strike_price_currency;
  InstrumentClass instrument_class;
  std::array<char, 2> _reserved4;
  std::int64_t strike_price;
  std::array<char, 6> _reserved5;
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
  // padding for alignment
  std::array<char, 3> dummy;
};
static_assert(sizeof(InstrumentDefMsg) == 360, "Size must match Rust");
static_assert(alignof(InstrumentDefMsg) == 8, "Must have 8-byte alignment");

// An error message from the Live Subscription Gateway (LSG). This will never
// be present in historical data.
struct ErrorMsg {
  static bool HasRType(RType rtype) { return rtype == RType::Error; }

  v2::ErrorMsg ToV2() const;
  UnixNanos IndexTs() const { return hd.ts_event; }
  const char* Err() const { return err.data(); }

  RecordHeader hd;
  std::array<char, 64> err;
};
static_assert(sizeof(ErrorMsg) == 80, "ErrorMsg size must match Rust");
static_assert(alignof(ErrorMsg) == 8, "Must have 8-byte alignment");

/// A symbol mapping message.
struct SymbolMappingMsg {
  static bool HasRType(RType rtype) { return rtype == RType::SymbolMapping; }

  v2::SymbolMappingMsg ToV2() const;
  const char* STypeInSymbol() const { return stype_in_symbol.data(); }
  const char* STypeOutSymbol() const { return stype_out_symbol.data(); }

  RecordHeader hd;
  std::array<char, kSymbolCstrLen> stype_in_symbol;
  std::array<char, kSymbolCstrLen> stype_out_symbol;
  // padding for alignment
  std::array<char, 4> dummy;
  UnixNanos start_ts;
  UnixNanos end_ts;
};
static_assert(sizeof(SymbolMappingMsg) == 80, "Size must match Rust");
static_assert(alignof(SymbolMappingMsg) == 8, "Must have 8-byte alignment");

struct SystemMsg {
  static bool HasRType(RType rtype) { return rtype == RType::System; }

  v2::SystemMsg ToV2() const;
  UnixNanos IndexTs() const { return hd.ts_event; }
  const char* Msg() const { return msg.data(); }
  bool IsHeartbeat() const {
    return std::strncmp(msg.data(), "Heartbeat", 9) == 0;
  }

  RecordHeader hd;
  std::array<char, 64> msg;
};
static_assert(sizeof(SystemMsg) == 80, "SystemMsg size must match Rust");
static_assert(alignof(SystemMsg) == 8, "Must have 8-byte alignment");

bool operator==(const InstrumentDefMsg& lhs, const InstrumentDefMsg& rhs);
inline bool operator!=(const InstrumentDefMsg& lhs,
                       const InstrumentDefMsg& rhs) {
  return !(lhs == rhs);
}
inline bool operator==(const ErrorMsg& lhs, const ErrorMsg& rhs) {
  return std::tie(lhs.hd, lhs.err) == std::tie(rhs.hd, rhs.err);
}
inline bool operator!=(const ErrorMsg& lhs, const ErrorMsg& rhs) {
  return !(lhs == rhs);
}
inline bool operator==(const SymbolMappingMsg& lhs,
                       const SymbolMappingMsg& rhs) {
  return std::tie(lhs.hd, lhs.stype_in_symbol, lhs.stype_out_symbol,
                  lhs.start_ts, lhs.end_ts) ==
         std::tie(rhs.hd, rhs.stype_in_symbol, rhs.stype_out_symbol,
                  rhs.start_ts, rhs.end_ts);
}
inline bool operator!=(const SymbolMappingMsg& lhs,
                       const SymbolMappingMsg& rhs) {
  return !(lhs == rhs);
}
inline bool operator==(const SystemMsg& lhs, const SystemMsg& rhs) {
  return std::tie(lhs.hd, lhs.msg) == std::tie(rhs.hd, rhs.msg);
}
inline bool operator!=(const SystemMsg& lhs, const SystemMsg& rhs) {
  return !(lhs == rhs);
}
std::string ToString(const InstrumentDefMsg& instr_def_msg);
std::ostream& operator<<(std::ostream& stream,
                         const InstrumentDefMsg& instr_def_msg);
std::string ToString(const ErrorMsg& err_msg);
std::ostream& operator<<(std::ostream& stream, const ErrorMsg& err_msg);
std::string ToString(const SymbolMappingMsg& symbol_mapping_msg);
std::ostream& operator<<(std::ostream& stream,
                         const SymbolMappingMsg& symbol_mapping_msg);
std::string ToString(const SystemMsg& sys_msg);
std::ostream& operator<<(std::ostream& stream, const SystemMsg& sys_msg);
}  // namespace v1
}  // namespace databento
