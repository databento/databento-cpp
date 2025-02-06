#pragma once

#include <cstddef>
#include <cstdint>

#include "databento/constants.hpp"  // kSymbolCstrLen
#include "databento/datetime.hpp"   // UnixNanos
#include "databento/enums.hpp"  // InstrumentClass, MatchingAlgorithm, RType, SecurityUpdateAction, Side, UserDefinedInstrument
#include "databento/record.hpp"  // RecordHeader

namespace databento::v3 {
static constexpr std::size_t kSymbolCstrLen = databento::kSymbolCstrLen;

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
using ErrorMsg = databento::ErrorMsg;
using SymbolMappingMsg = databento::SymbolMappingMsg;
using SystemMsg = databento::SystemMsg;

// An instrument definition in DBN version 3.
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
  const char* StrikePriceCurrency() const {
    return strike_price_currency.data();
  }
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
  std::array<char, 7> asset;
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
  // padding for alignment
  std::array<char, 21> reserved;
};
static_assert(sizeof(InstrumentDefMsg) == 520,
              "InstrumentDefMsg size must match Rust");
static_assert(alignof(InstrumentDefMsg) == 8, "Must have 8-byte alignment");
static_assert(kMaxRecordLen == sizeof(InstrumentDefMsg) + sizeof(UnixNanos),
              "v3 definition with ts_out should be the largest record");

bool operator==(const InstrumentDefMsg& lhs, const InstrumentDefMsg& rhs);
inline bool operator!=(const InstrumentDefMsg& lhs,
                       const InstrumentDefMsg& rhs) {
  return !(lhs == rhs);
}

std::string ToString(const InstrumentDefMsg& instr_def_msg);
std::ostream& operator<<(std::ostream& stream,
                         const InstrumentDefMsg& instr_def_msg);
}  // namespace databento::v3
