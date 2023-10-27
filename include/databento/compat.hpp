// Record definitions from previous DBN versions and helper functions.
#pragma once

#include <cstddef>  // size_t
#include <cstdint>

#include "databento/constants.hpp"  // kSymbolCstrLen
#include "databento/datetime.hpp"   // UnixNanos
#include "databento/enums.hpp"
#include "databento/record.hpp"

namespace databento {
static constexpr std::size_t kSymbolCstrLenV1 = 22;
static constexpr std::size_t kSymbolCstrLenV2 = kSymbolCstrLen;

constexpr std::size_t VersionSymbolCstrLen(std::uint8_t version) {
  if (version < 2) {
    return kSymbolCstrLenV1;
  }
  return kSymbolCstrLenV2;
}

using InstrumentDefMsgV2 = InstrumentDefMsg;
using SymbolMappingMsgV2 = SymbolMappingMsg;

// DBN version 1 instrument definition.
struct InstrumentDefMsgV1 {
  static bool HasRType(RType rtype) { return rtype == RType::InstrumentDef; }

  InstrumentDefMsgV2 ToV2() const;
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
  std::array<char, kSymbolCstrLenV1> raw_symbol;
  std::array<char, 21> group;
  std::array<char, 5> exchange;
  std::array<char, 7> asset;
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
static_assert(sizeof(InstrumentDefMsgV1) == 360);

/// A symbol mapping message.
struct SymbolMappingMsgV1 {
  static bool HasRType(RType rtype) { return rtype == RType::SymbolMapping; }

  SymbolMappingMsgV2 ToV2() const;
  const char* STypeInSymbol() const { return stype_in_symbol.data(); }
  const char* STypeOutSymbol() const { return stype_out_symbol.data(); }

  RecordHeader hd;
  std::array<char, kSymbolCstrLenV1> stype_in_symbol;
  std::array<char, kSymbolCstrLenV1> stype_out_symbol;
  // padding for alignment
  std::array<char, 4> dummy;
  UnixNanos start_ts;
  UnixNanos end_ts;
};
static_assert(sizeof(SymbolMappingMsgV1) == 80);

bool operator==(const InstrumentDefMsgV1& lhs, const InstrumentDefMsgV1& rhs);
inline bool operator!=(const InstrumentDefMsgV1& lhs,
                       const InstrumentDefMsgV1& rhs) {
  return !(lhs == rhs);
}
inline bool operator==(const SymbolMappingMsgV1& lhs,
                       const SymbolMappingMsgV1& rhs) {
  return std::tie(lhs.hd, lhs.stype_in_symbol, lhs.stype_out_symbol,
                  lhs.start_ts, lhs.end_ts) ==
         std::tie(rhs.hd, rhs.stype_in_symbol, rhs.stype_out_symbol,
                  rhs.start_ts, rhs.end_ts);
}
inline bool operator!=(const SymbolMappingMsgV1& lhs,
                       const SymbolMappingMsgV1& rhs) {
  return !(lhs == rhs);
}
std::string ToString(const InstrumentDefMsgV1& instr_def_msg);
std::ostream& operator<<(std::ostream& stream,
                         const InstrumentDefMsgV1& instr_def_msg);
std::string ToString(const SymbolMappingMsgV1& symbol_mapping_msg);
std::ostream& operator<<(std::ostream& stream,
                         const SymbolMappingMsgV1& symbol_mapping_msg);
}  // namespace databento
