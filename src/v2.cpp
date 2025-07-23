#include "databento/v2.hpp"

#include "databento/pretty.hpp"  // Px
#include "databento/v3.hpp"
#include "stream_op_helper.hpp"

namespace databento::v2 {
databento::v3::InstrumentDefMsg InstrumentDefMsg::ToV3() const {
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

template <>
v3::InstrumentDefMsg InstrumentDefMsg::Upgrade() const {
  return ToV3();
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
         lhs.price_ratio == rhs.price_ratio && lhs.strike_price == rhs.strike_price &&
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

std::string ToString(const InstrumentDefMsg& instr_def_msg) {
  return MakeString(instr_def_msg);
}
std::ostream& operator<<(std::ostream& stream, const InstrumentDefMsg& instr_def_msg) {
  return StreamOpBuilder{stream}
      .SetSpacer("\n    ")
      .SetTypeName("v2::InstrumentDefMsg")
      .Build()
      .AddField("hd", instr_def_msg.hd)
      .AddField("ts_recv", instr_def_msg.ts_recv)
      .AddField("min_price_increment", pretty::Px{instr_def_msg.min_price_increment})
      .AddField("display_factor", pretty::Px{instr_def_msg.display_factor})
      .AddField("expiration", instr_def_msg.expiration)
      .AddField("activation", instr_def_msg.activation)
      .AddField("high_limit_price", pretty::Px{instr_def_msg.high_limit_price})
      .AddField("low_limit_price", pretty::Px{instr_def_msg.low_limit_price})
      .AddField("max_price_variation", pretty::Px{instr_def_msg.max_price_variation})
      .AddField("trading_reference_price",
                pretty::Px{instr_def_msg.trading_reference_price})
      .AddField("unit_of_measure_qty", pretty::Px{instr_def_msg.unit_of_measure_qty})
      .AddField("min_price_increment_amount",
                pretty::Px{instr_def_msg.min_price_increment_amount})
      .AddField("price_ratio", pretty::Px{instr_def_msg.price_ratio})
      .AddField("strike_price", pretty::Px{instr_def_msg.strike_price})
      .AddField("inst_attrib_value", instr_def_msg.inst_attrib_value)
      .AddField("underlying_id", instr_def_msg.underlying_id)
      .AddField("raw_instrument_id", instr_def_msg.raw_instrument_id)
      .AddField("market_depth_implied", instr_def_msg.market_depth_implied)
      .AddField("market_depth", instr_def_msg.market_depth)
      .AddField("market_segment_id", instr_def_msg.market_segment_id)
      .AddField("max_trade_vol", instr_def_msg.max_trade_vol)
      .AddField("min_lot_size", instr_def_msg.min_lot_size)
      .AddField("min_lot_size_block", instr_def_msg.min_lot_size_block)
      .AddField("min_lot_size_round_lot", instr_def_msg.min_lot_size_round_lot)
      .AddField("min_trade_vol", instr_def_msg.min_trade_vol)
      .AddField("contract_multiplier", instr_def_msg.contract_multiplier)
      .AddField("decay_quantity", instr_def_msg.decay_quantity)
      .AddField("original_contract_size", instr_def_msg.original_contract_size)
      .AddField("trading_reference_date", instr_def_msg.trading_reference_date)
      .AddField("appl_id", instr_def_msg.appl_id)
      .AddField("maturity_year", instr_def_msg.maturity_year)
      .AddField("decay_start_date", instr_def_msg.decay_start_date)
      .AddField("channel_id", instr_def_msg.channel_id)
      .AddField("currency", instr_def_msg.currency)
      .AddField("settl_currency", instr_def_msg.settl_currency)
      .AddField("secsubtype", instr_def_msg.secsubtype)
      .AddField("raw_symbol", instr_def_msg.raw_symbol)
      .AddField("group", instr_def_msg.group)
      .AddField("exchange", instr_def_msg.exchange)
      .AddField("asset", instr_def_msg.asset)
      .AddField("cfi", instr_def_msg.cfi)
      .AddField("security_type", instr_def_msg.security_type)
      .AddField("unit_of_measure", instr_def_msg.unit_of_measure)
      .AddField("underlying", instr_def_msg.underlying)
      .AddField("strike_price_currency", instr_def_msg.strike_price_currency)
      .AddField("instrument_class", instr_def_msg.instrument_class)
      .AddField("match_algorithm", instr_def_msg.match_algorithm)
      .AddField("md_security_trading_status", instr_def_msg.md_security_trading_status)
      .AddField("main_fraction", instr_def_msg.main_fraction)
      .AddField("price_display_format", instr_def_msg.price_display_format)
      .AddField("settl_price_type", instr_def_msg.settl_price_type)
      .AddField("sub_fraction", instr_def_msg.sub_fraction)
      .AddField("underlying_product", instr_def_msg.underlying_product)
      .AddField("security_update_action", instr_def_msg.security_update_action)
      .AddField("maturity_month", instr_def_msg.maturity_month)
      .AddField("maturity_day", instr_def_msg.maturity_day)
      .AddField("maturity_week", instr_def_msg.maturity_week)
      .AddField("user_defined_instrument", instr_def_msg.user_defined_instrument)
      .AddField("contract_multiplier_unit", instr_def_msg.contract_multiplier_unit)
      .AddField("flow_schedule_type", instr_def_msg.flow_schedule_type)
      .AddField("tick_rule", instr_def_msg.tick_rule)
      .Finish();
}

}  // namespace databento::v2
