#include "databento/v3.hpp"

#include "databento/fixed_price.hpp"
#include "stream_op_helper.hpp"

namespace databento::v3 {

bool operator==(const InstrumentDefMsg& lhs, const InstrumentDefMsg& rhs) {
  return lhs.hd == rhs.hd && lhs.ts_recv == rhs.ts_recv &&
         lhs.min_price_increment == rhs.min_price_increment &&
         lhs.display_factor == rhs.display_factor &&
         lhs.expiration == rhs.expiration && lhs.activation == rhs.activation &&
         lhs.high_limit_price == rhs.high_limit_price &&
         lhs.low_limit_price == rhs.low_limit_price &&
         lhs.max_price_variation == rhs.max_price_variation &&
         lhs.unit_of_measure_qty == rhs.unit_of_measure_qty &&
         lhs.min_price_increment_amount == rhs.min_price_increment_amount &&
         lhs.price_ratio == rhs.price_ratio &&
         lhs.strike_price == rhs.strike_price &&
         lhs.raw_instrument_id == rhs.raw_instrument_id &&
         lhs.leg_price == rhs.leg_price && lhs.leg_delta == rhs.leg_delta &&
         lhs.inst_attrib_value == rhs.inst_attrib_value &&
         lhs.underlying_id == rhs.underlying_id &&
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
         lhs.leg_instrument_id == rhs.leg_instrument_id &&
         lhs.leg_ratio_price_numerator == rhs.leg_ratio_price_numerator &&
         lhs.leg_ratio_price_denominator == rhs.leg_ratio_price_denominator &&
         lhs.leg_ratio_qty_numerator == rhs.leg_ratio_qty_numerator &&
         lhs.leg_ratio_qty_denominator == rhs.leg_ratio_qty_denominator &&
         lhs.leg_underlying_id == rhs.leg_underlying_id &&
         lhs.appl_id == rhs.appl_id && lhs.maturity_year == rhs.maturity_year &&
         lhs.decay_start_date == rhs.decay_start_date &&
         lhs.channel_id == rhs.channel_id && lhs.leg_count == rhs.leg_count &&
         lhs.leg_index == rhs.leg_index && lhs.currency == rhs.currency &&
         lhs.settl_currency == rhs.settl_currency &&
         lhs.secsubtype == rhs.secsubtype && lhs.raw_symbol == rhs.raw_symbol &&
         lhs.group == rhs.group && lhs.exchange == rhs.exchange &&
         lhs.asset == rhs.asset && lhs.cfi == rhs.cfi &&
         lhs.security_type == rhs.security_type &&
         lhs.unit_of_measure == rhs.unit_of_measure &&
         lhs.underlying == rhs.underlying &&
         lhs.strike_price_currency == rhs.strike_price_currency &&
         lhs.leg_raw_symbol == rhs.leg_raw_symbol &&
         lhs.instrument_class == rhs.instrument_class &&
         lhs.match_algorithm == rhs.match_algorithm &&
         lhs.main_fraction == rhs.main_fraction &&
         lhs.price_display_format == rhs.price_display_format &&
         lhs.sub_fraction == rhs.sub_fraction &&
         lhs.underlying_product == rhs.underlying_product &&
         lhs.security_update_action == rhs.security_update_action &&
         lhs.maturity_month == rhs.maturity_month &&
         lhs.maturity_day == rhs.maturity_day &&
         lhs.maturity_week == rhs.maturity_week &&
         lhs.user_defined_instrument == rhs.user_defined_instrument &&
         lhs.contract_multiplier_unit == rhs.contract_multiplier_unit &&
         lhs.flow_schedule_type == rhs.flow_schedule_type &&
         lhs.tick_rule == rhs.tick_rule &&
         lhs.leg_instrument_class == rhs.leg_instrument_class &&
         lhs.leg_side == rhs.leg_side;
}

std::string ToString(const InstrumentDefMsg& instr_def_msg) {
  return MakeString(instr_def_msg);
}
std::ostream& operator<<(std::ostream& stream,
                         const InstrumentDefMsg& instr_def_msg) {
  return StreamOpBuilder{stream}
      .SetSpacer("\n    ")
      .SetTypeName("v1::InstrumentDefMsg")
      .Build()
      .AddField("hd", instr_def_msg.hd)
      .AddField("ts_recv", instr_def_msg.ts_recv)
      .AddField("min_price_increment", FixPx{instr_def_msg.min_price_increment})
      .AddField("display_factor", FixPx{instr_def_msg.display_factor})
      .AddField("expiration", instr_def_msg.expiration)
      .AddField("activation", instr_def_msg.activation)
      .AddField("high_limit_price", FixPx{instr_def_msg.high_limit_price})
      .AddField("low_limit_price", FixPx{instr_def_msg.low_limit_price})
      .AddField("max_price_variation", FixPx{instr_def_msg.max_price_variation})
      .AddField("unit_of_measure_qty", FixPx{instr_def_msg.unit_of_measure_qty})
      .AddField("min_price_increment_amount",
                FixPx{instr_def_msg.min_price_increment_amount})
      .AddField("price_ratio", FixPx{instr_def_msg.price_ratio})
      .AddField("strike_price", FixPx{instr_def_msg.strike_price})
      .AddField("raw_instrument_id", instr_def_msg.raw_instrument_id)
      .AddField("leg_price", FixPx{instr_def_msg.leg_price})
      .AddField("leg_delta", FixPx{instr_def_msg.leg_delta})
      .AddField("inst_attrib_value", instr_def_msg.inst_attrib_value)
      .AddField("underlying_id", instr_def_msg.underlying_id)
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
      .AddField("leg_instrument_id", instr_def_msg.leg_instrument_id)
      .AddField("leg_ratio_price_numerator",
                instr_def_msg.leg_ratio_price_numerator)
      .AddField("leg_ratio_price_denominator",
                instr_def_msg.leg_ratio_price_denominator)
      .AddField("leg_ratio_qty_numerator",
                instr_def_msg.leg_ratio_qty_numerator)
      .AddField("leg_ratio_qty_denominator",
                instr_def_msg.leg_ratio_qty_denominator)
      .AddField("leg_underlying_id", instr_def_msg.leg_underlying_id)
      .AddField("appl_id", instr_def_msg.appl_id)
      .AddField("maturity_year", instr_def_msg.maturity_year)
      .AddField("decay_start_date", instr_def_msg.decay_start_date)
      .AddField("channel_id", instr_def_msg.channel_id)
      .AddField("leg_count", instr_def_msg.leg_count)
      .AddField("leg_index", instr_def_msg.leg_index)
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
      .AddField("leg_raw_symbol", instr_def_msg.leg_raw_symbol)
      .AddField("instrument_class", instr_def_msg.instrument_class)
      .AddField("match_algorithm", instr_def_msg.match_algorithm)
      .AddField("main_fraction", instr_def_msg.main_fraction)
      .AddField("price_display_format", instr_def_msg.price_display_format)
      .AddField("sub_fraction", instr_def_msg.sub_fraction)
      .AddField("underlying_product", instr_def_msg.underlying_product)
      .AddField("security_update_action", instr_def_msg.security_update_action)
      .AddField("maturity_month", instr_def_msg.maturity_month)
      .AddField("maturity_day", instr_def_msg.maturity_day)
      .AddField("maturity_week", instr_def_msg.maturity_week)
      .AddField("user_defined_instrument",
                instr_def_msg.user_defined_instrument)
      .AddField("contract_multiplier_unit",
                instr_def_msg.contract_multiplier_unit)
      .AddField("flow_schedule_type", instr_def_msg.flow_schedule_type)
      .AddField("tick_rule", instr_def_msg.tick_rule)
      .AddField("leg_instrument_class", instr_def_msg.leg_instrument_class)
      .AddField("leg_side", instr_def_msg.leg_side)
      .Finish();
}
}  // namespace databento::v3
