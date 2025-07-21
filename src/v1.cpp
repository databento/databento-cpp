#include "databento/v1.hpp"

#include <algorithm>  // copy
#include <cstdint>
#include <limits>  // numeric_limits

#include "databento/enums.hpp"
#include "databento/pretty.hpp"  // Px
#include "databento/record.hpp"
#include "databento/v2.hpp"
#include "databento/v3.hpp"
#include "stream_op_helper.hpp"  // MakeString, StreamOpBuilder

namespace databento::v1 {
v2::InstrumentDefMsg InstrumentDefMsg::ToV2() const {
  v2::InstrumentDefMsg ret{
      RecordHeader{sizeof(v2::InstrumentDefMsg) / RecordHeader::kLengthMultiplier,
                   RType::InstrumentDef, hd.publisher_id, hd.instrument_id,
                   hd.ts_event},
      ts_recv,
      min_price_increment,
      display_factor,
      expiration,
      activation,
      high_limit_price,
      low_limit_price,
      max_price_variation,
      trading_reference_price,
      unit_of_measure_qty,
      min_price_increment_amount,
      price_ratio,
      strike_price,
      inst_attrib_value,
      underlying_id,
      raw_instrument_id,
      market_depth_implied,
      market_depth,
      market_segment_id,
      max_trade_vol,
      min_lot_size,
      min_lot_size_block,
      min_lot_size_round_lot,
      min_trade_vol,
      contract_multiplier,
      decay_quantity,
      original_contract_size,
      trading_reference_date,
      appl_id,
      maturity_year,
      decay_start_date,
      channel_id,
      currency,
      settl_currency,
      {},
      {},
      {},
      {},
      {},
      {},
      {},
      {},
      {},
      {},
      instrument_class,
      match_algorithm,
      md_security_trading_status,
      main_fraction,
      price_display_format,
      settl_price_type,
      sub_fraction,
      underlying_product,
      security_update_action,
      maturity_month,
      maturity_day,
      maturity_week,
      user_defined_instrument,
      contract_multiplier_unit,
      flow_schedule_type,
      tick_rule,
      {}};
  std::copy(currency.begin(), currency.end(), ret.currency.begin());
  std::copy(settl_currency.begin(), settl_currency.end(), ret.settl_currency.begin());
  std::copy(secsubtype.begin(), secsubtype.end(), ret.secsubtype.begin());
  std::copy(raw_symbol.begin(), raw_symbol.end(), ret.raw_symbol.begin());
  std::copy(group.begin(), group.end(), ret.group.begin());
  std::copy(exchange.begin(), exchange.end(), ret.exchange.begin());
  std::copy(asset.begin(), asset.end(), ret.asset.begin());
  std::copy(cfi.begin(), cfi.end(), ret.cfi.begin());
  std::copy(security_type.begin(), security_type.end(), ret.security_type.begin());
  std::copy(unit_of_measure.begin(), unit_of_measure.end(),
            ret.unit_of_measure.begin());
  std::copy(underlying.begin(), underlying.end(), ret.underlying.begin());
  std::copy(strike_price_currency.begin(), strike_price_currency.end(),
            ret.strike_price_currency.begin());
  return ret;
}

v3::InstrumentDefMsg InstrumentDefMsg::ToV3() const {
  v3::InstrumentDefMsg ret{
      RecordHeader{sizeof(v3::InstrumentDefMsg) / RecordHeader::kLengthMultiplier,
                   RType::InstrumentDef, hd.publisher_id, hd.instrument_id,
                   hd.ts_event},
      ts_recv,
      min_price_increment,
      display_factor,
      expiration,
      activation,
      high_limit_price,
      low_limit_price,
      max_price_variation,
      unit_of_measure_qty,
      min_price_increment_amount,
      price_ratio,
      strike_price,
      raw_instrument_id,
      kUndefPrice,
      kUndefPrice,
      inst_attrib_value,
      underlying_id,
      market_depth_implied,
      market_depth,
      market_segment_id,
      max_trade_vol,
      min_lot_size,
      min_lot_size_block,
      min_lot_size_round_lot,
      min_trade_vol,
      contract_multiplier,
      decay_quantity,
      original_contract_size,
      {},
      {},
      {},
      {},
      {},
      {},
      appl_id,
      maturity_year,
      decay_start_date,
      channel_id,
      {},
      {},
      currency,
      settl_currency,
      secsubtype,
      {},
      {},
      {},
      {},
      {},
      {},
      {},
      {},
      {},
      {},
      instrument_class,
      match_algorithm,
      main_fraction,
      price_display_format,
      sub_fraction,
      underlying_product,
      security_update_action,
      maturity_month,
      maturity_day,
      maturity_week,
      user_defined_instrument,
      contract_multiplier_unit,
      flow_schedule_type,
      tick_rule,
      {},
      Side::None,
      {}};
  std::copy(raw_symbol.begin(), raw_symbol.end(), ret.raw_symbol.begin());
  std::copy(group.begin(), group.end(), ret.group.begin());
  std::copy(exchange.begin(), exchange.end(), ret.exchange.begin());
  std::copy(asset.begin(), asset.end(), ret.asset.begin());
  std::copy(cfi.begin(), cfi.end(), ret.cfi.begin());
  std::copy(security_type.begin(), security_type.end(), ret.security_type.begin());
  std::copy(unit_of_measure.begin(), unit_of_measure.end(),
            ret.unit_of_measure.begin());
  std::copy(underlying.begin(), underlying.end(), ret.underlying.begin());
  std::copy(strike_price_currency.begin(), strike_price_currency.end(),
            ret.strike_price_currency.begin());
  return ret;
}

v3::StatMsg StatMsg::ToV3() const {
  return v3::StatMsg{
      RecordHeader{sizeof(v3::StatMsg) / RecordHeader::kLengthMultiplier,
                   RType::Statistics, hd.publisher_id, hd.instrument_id, hd.ts_event},
      ts_recv,
      ts_ref,
      price,
      quantity == kUndefStatQuantity ? v3::kUndefStatQuantity : quantity,
      sequence,
      ts_in_delta,
      stat_type,
      channel_id,
      update_action,
      stat_flags,
      {}};
}

v2::ErrorMsg ErrorMsg::ToV2() const {
  v2::ErrorMsg ret{
      RecordHeader{sizeof(v2::ErrorMsg) / RecordHeader::kLengthMultiplier, RType::Error,
                   hd.publisher_id, hd.instrument_id, hd.ts_event},
      {},
      ErrorCode::Unset,
      std::numeric_limits<std::uint8_t>::max()};
  std::copy(err.begin(), err.end(), ret.err.begin());
  return ret;
}

v2::SymbolMappingMsg SymbolMappingMsg::ToV2() const {
  v2::SymbolMappingMsg ret{
      RecordHeader{sizeof(v2::SymbolMappingMsg) / RecordHeader::kLengthMultiplier,
                   RType::SymbolMapping, hd.publisher_id, hd.instrument_id,
                   hd.ts_event},
      // Intentionally invalid
      // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange)
      static_cast<SType>(std::numeric_limits<std::uint8_t>::max()),
      {},
      // Intentionally invalid
      // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange)
      static_cast<SType>(std::numeric_limits<std::uint8_t>::max()),
      {},
      start_ts,
      end_ts};
  std::copy(stype_in_symbol.begin(), stype_in_symbol.end(),
            ret.stype_in_symbol.begin());
  std::copy(stype_out_symbol.begin(), stype_out_symbol.end(),
            ret.stype_out_symbol.begin());
  return ret;
}

v2::SystemMsg SystemMsg::ToV2() const {
  v2::SystemMsg ret{
      RecordHeader{sizeof(v2::SystemMsg) / RecordHeader::kLengthMultiplier,
                   RType::System, hd.publisher_id, hd.instrument_id, hd.ts_event},
      {},
      IsHeartbeat() ? SystemCode::Heartbeat : SystemCode::Unset};
  std::copy(msg.begin(), msg.end(), ret.msg.begin());
  return ret;
}

template <>
v2::ErrorMsg ErrorMsg::Upgrade() const {
  return ToV2();
}

std::string ToString(const ErrorMsg& error_msg) { return MakeString(error_msg); }
std::ostream& operator<<(std::ostream& stream, const ErrorMsg& error_msg) {
  return StreamOpBuilder{stream}
      .SetSpacer("\n    ")
      .SetTypeName("ErrorMsg")
      .Build()
      .AddField("hd", error_msg.hd)
      .AddField("err", error_msg.err)
      .Finish();
}

bool operator==(const InstrumentDefMsg& lhs, const InstrumentDefMsg& rhs) {
  return lhs.hd == rhs.hd && lhs.ts_recv == rhs.ts_recv &&
         lhs.min_price_increment == rhs.min_price_increment &&
         lhs.display_factor == rhs.display_factor && lhs.expiration == rhs.expiration &&
         lhs.activation == rhs.activation &&
         lhs.high_limit_price == rhs.high_limit_price &&
         lhs.low_limit_price == rhs.low_limit_price &&
         lhs.max_price_variation == rhs.max_price_variation &&
         lhs.trading_reference_price == rhs.trading_reference_price &&
         lhs.unit_of_measure_qty == rhs.unit_of_measure_qty &&
         lhs.min_price_increment_amount == rhs.min_price_increment_amount &&
         lhs.price_ratio == rhs.price_ratio &&
         lhs.inst_attrib_value == rhs.inst_attrib_value &&
         lhs.underlying_id == rhs.underlying_id &&
         lhs.raw_instrument_id == rhs.raw_instrument_id &&
         lhs.market_depth_implied == rhs.market_depth_implied &&
         lhs.market_depth == rhs.market_depth &&
         lhs.market_segment_id == rhs.market_segment_id &&
         lhs.max_trade_vol == rhs.max_trade_vol &&
         lhs.min_lot_size == rhs.min_lot_size &&
         lhs.min_lot_size_block == rhs.min_lot_size_block &&
         lhs.min_lot_size_round_lot == rhs.min_lot_size_round_lot &&
         lhs.min_trade_vol == rhs.min_trade_vol &&
         lhs.contract_multiplier == rhs.contract_multiplier &&
         lhs.decay_quantity == rhs.decay_quantity &&
         lhs.original_contract_size == rhs.original_contract_size &&
         lhs.trading_reference_date == rhs.trading_reference_date &&
         lhs.appl_id == rhs.appl_id && lhs.maturity_year == rhs.maturity_year &&
         lhs.decay_start_date == rhs.decay_start_date &&
         lhs.channel_id == rhs.channel_id && lhs.currency == rhs.currency &&
         lhs.settl_currency == rhs.settl_currency && lhs.secsubtype == rhs.secsubtype &&
         lhs.raw_symbol == rhs.raw_symbol && lhs.group == rhs.group &&
         lhs.exchange == rhs.exchange && lhs.asset == rhs.asset && lhs.cfi == rhs.cfi &&
         lhs.security_type == rhs.security_type &&
         lhs.unit_of_measure == rhs.unit_of_measure &&
         lhs.underlying == rhs.underlying &&
         lhs.strike_price_currency == rhs.strike_price_currency &&
         lhs.instrument_class == rhs.instrument_class &&
         lhs.strike_price == rhs.strike_price &&
         lhs.match_algorithm == rhs.match_algorithm &&
         lhs.md_security_trading_status == rhs.md_security_trading_status &&
         lhs.main_fraction == rhs.main_fraction &&
         lhs.price_display_format == rhs.price_display_format &&
         lhs.settl_price_type == rhs.settl_price_type &&
         lhs.sub_fraction == rhs.sub_fraction &&
         lhs.underlying_product == rhs.underlying_product &&
         lhs.security_update_action == rhs.security_update_action &&
         lhs.maturity_month == rhs.maturity_month &&
         lhs.maturity_day == rhs.maturity_day &&
         lhs.maturity_week == rhs.maturity_week &&
         lhs.user_defined_instrument == rhs.user_defined_instrument &&
         lhs.contract_multiplier_unit == rhs.contract_multiplier_unit &&
         lhs.flow_schedule_type == rhs.flow_schedule_type &&
         lhs.tick_rule == rhs.tick_rule;
}

template <>
v2::InstrumentDefMsg InstrumentDefMsg::Upgrade() const {
  return ToV2();
}

template <>
v3::InstrumentDefMsg InstrumentDefMsg::Upgrade() const {
  return ToV3();
}

std::string ToString(const InstrumentDefMsg& instrument_def_msg) {
  return MakeString(instrument_def_msg);
}
std::ostream& operator<<(std::ostream& stream,
                         const InstrumentDefMsg& instrument_def_msg) {
  return StreamOpBuilder{stream}
      .SetSpacer("\n    ")
      .SetTypeName("InstrumentDefMsg")
      .Build()
      .AddField("hd", instrument_def_msg.hd)
      .AddField("ts_recv", instrument_def_msg.ts_recv)
      .AddField("min_price_increment",
                pretty::Px{instrument_def_msg.min_price_increment})
      .AddField("display_factor", pretty::Px{instrument_def_msg.display_factor})
      .AddField("expiration", instrument_def_msg.expiration)
      .AddField("activation", instrument_def_msg.activation)
      .AddField("high_limit_price", pretty::Px{instrument_def_msg.high_limit_price})
      .AddField("low_limit_price", pretty::Px{instrument_def_msg.low_limit_price})
      .AddField("max_price_variation",
                pretty::Px{instrument_def_msg.max_price_variation})
      .AddField("trading_reference_price",
                pretty::Px{instrument_def_msg.trading_reference_price})
      .AddField("unit_of_measure_qty",
                pretty::Px{instrument_def_msg.unit_of_measure_qty})
      .AddField("min_price_increment_amount",
                pretty::Px{instrument_def_msg.min_price_increment_amount})
      .AddField("price_ratio", pretty::Px{instrument_def_msg.price_ratio})
      .AddField("inst_attrib_value", instrument_def_msg.inst_attrib_value)
      .AddField("underlying_id", instrument_def_msg.underlying_id)
      .AddField("raw_instrument_id", instrument_def_msg.raw_instrument_id)
      .AddField("market_depth_implied", instrument_def_msg.market_depth_implied)
      .AddField("market_depth", instrument_def_msg.market_depth)
      .AddField("market_segment_id", instrument_def_msg.market_segment_id)
      .AddField("max_trade_vol", instrument_def_msg.max_trade_vol)
      .AddField("min_lot_size", instrument_def_msg.min_lot_size)
      .AddField("min_lot_size_block", instrument_def_msg.min_lot_size_block)
      .AddField("min_lot_size_round_lot", instrument_def_msg.min_lot_size_round_lot)
      .AddField("min_trade_vol", instrument_def_msg.min_trade_vol)
      .AddField("contract_multiplier", instrument_def_msg.contract_multiplier)
      .AddField("decay_quantity", instrument_def_msg.decay_quantity)
      .AddField("original_contract_size", instrument_def_msg.original_contract_size)
      .AddField("trading_reference_date", instrument_def_msg.trading_reference_date)
      .AddField("appl_id", instrument_def_msg.appl_id)
      .AddField("maturity_year", instrument_def_msg.maturity_year)
      .AddField("decay_start_date", instrument_def_msg.decay_start_date)
      .AddField("channel_id", instrument_def_msg.channel_id)
      .AddField("currency", instrument_def_msg.currency)
      .AddField("settl_currency", instrument_def_msg.settl_currency)
      .AddField("secsubtype", instrument_def_msg.secsubtype)
      .AddField("raw_symbol", instrument_def_msg.raw_symbol)
      .AddField("group", instrument_def_msg.group)
      .AddField("exchange", instrument_def_msg.exchange)
      .AddField("asset", instrument_def_msg.asset)
      .AddField("cfi", instrument_def_msg.cfi)
      .AddField("security_type", instrument_def_msg.security_type)
      .AddField("unit_of_measure", instrument_def_msg.unit_of_measure)
      .AddField("underlying", instrument_def_msg.underlying)
      .AddField("strike_price_currency", instrument_def_msg.strike_price_currency)
      .AddField("instrument_class", instrument_def_msg.instrument_class)
      .AddField("strike_price", pretty::Px{instrument_def_msg.strike_price})
      .AddField("match_algorithm", instrument_def_msg.match_algorithm)
      .AddField("md_security_trading_status",
                instrument_def_msg.md_security_trading_status)
      .AddField("main_fraction", instrument_def_msg.main_fraction)
      .AddField("price_display_format", instrument_def_msg.price_display_format)
      .AddField("settl_price_type", instrument_def_msg.settl_price_type)
      .AddField("sub_fraction", instrument_def_msg.sub_fraction)
      .AddField("underlying_product", instrument_def_msg.underlying_product)
      .AddField("security_update_action", instrument_def_msg.security_update_action)
      .AddField("maturity_month", instrument_def_msg.maturity_month)
      .AddField("maturity_day", instrument_def_msg.maturity_day)
      .AddField("maturity_week", instrument_def_msg.maturity_week)
      .AddField("user_defined_instrument", instrument_def_msg.user_defined_instrument)
      .AddField("contract_multiplier_unit", instrument_def_msg.contract_multiplier_unit)
      .AddField("flow_schedule_type", instrument_def_msg.flow_schedule_type)
      .AddField("tick_rule", instrument_def_msg.tick_rule)
      .Finish();
}

template <>
v3::StatMsg StatMsg::Upgrade() const {
  return ToV3();
}

std::string ToString(const StatMsg& stat_msg) { return MakeString(stat_msg); }
std::ostream& operator<<(std::ostream& stream, const StatMsg& stat_msg) {
  return StreamOpBuilder{stream}
      .SetSpacer("\n    ")
      .SetTypeName("StatMsg")
      .Build()
      .AddField("hd", stat_msg.hd)
      .AddField("ts_recv", stat_msg.ts_recv)
      .AddField("ts_ref", stat_msg.ts_ref)
      .AddField("price", pretty::Px{stat_msg.price})
      .AddField("quantity", stat_msg.quantity)
      .AddField("sequence", stat_msg.sequence)
      .AddField("ts_in_delta", stat_msg.ts_in_delta)
      .AddField("stat_type", stat_msg.stat_type)
      .AddField("channel_id", stat_msg.channel_id)
      .AddField("update_action", stat_msg.update_action)
      .AddField("stat_flags", stat_msg.stat_flags)
      .Finish();
}

template <>
v2::SymbolMappingMsg SymbolMappingMsg::Upgrade() const {
  return ToV2();
}

std::string ToString(const SymbolMappingMsg& symbol_mapping_msg) {
  return MakeString(symbol_mapping_msg);
}
std::ostream& operator<<(std::ostream& stream,
                         const SymbolMappingMsg& symbol_mapping_msg) {
  return StreamOpBuilder{stream}
      .SetSpacer("\n    ")
      .SetTypeName("SymbolMappingMsg")
      .Build()
      .AddField("hd", symbol_mapping_msg.hd)
      .AddField("stype_in_symbol", symbol_mapping_msg.stype_in_symbol)
      .AddField("stype_out_symbol", symbol_mapping_msg.stype_out_symbol)
      .AddField("start_ts", symbol_mapping_msg.start_ts)
      .AddField("end_ts", symbol_mapping_msg.end_ts)
      .Finish();
}

template <>
v2::SystemMsg SystemMsg::Upgrade() const {
  return ToV2();
}

std::string ToString(const SystemMsg& system_msg) { return MakeString(system_msg); }
std::ostream& operator<<(std::ostream& stream, const SystemMsg& system_msg) {
  return StreamOpBuilder{stream}
      .SetSpacer("\n    ")
      .SetTypeName("SystemMsg")
      .Build()
      .AddField("hd", system_msg.hd)
      .AddField("msg", system_msg.msg)
      .Finish();
}

}  // namespace databento::v1
