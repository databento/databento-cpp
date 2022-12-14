#include "databento/record.hpp"

#include <string>

#include "databento/enums.hpp"
#include "databento/exceptions.hpp"  // InvalidArgumentError
#include "stream_op_helper.hpp"

using databento::Record;

std::size_t Record::Size() const { return Record::SizeOfType(record_->rtype); }

std::size_t Record::SizeOfType(const std::uint8_t rtype) {
  switch (rtype) {
    case MboMsg::kTypeId: {
      return sizeof(MboMsg);
    }
    case TradeMsg::kTypeId: {
      return sizeof(TradeMsg);
    }
    case Mbp1Msg::kTypeId: {
      return sizeof(Mbp1Msg);
    }
    case Mbp10Msg::kTypeId: {
      return sizeof(Mbp10Msg);
    }
    case OhlcvMsg::kTypeId: {
      return sizeof(OhlcvMsg);
    }
    case InstrumentDefMsg::kTypeId: {
      return sizeof(InstrumentDefMsg);
    }
    default: {
      throw InvalidArgumentError{
          "Record::SizeOfType", "rtype",
          "unknown value '" + std::to_string(rtype) + "'"};
    }
  }
}

std::uint8_t Record::TypeIdFromSchema(const Schema schema) {
  switch (schema) {
    case Schema::Mbo: {
      return MboMsg::kTypeId;
    }
    case Schema::Mbp1: {
      return Mbp1Msg::kTypeId;
    }
    case Schema::Mbp10: {
      return Mbp10Msg::kTypeId;
    }
    case Schema::Trades: {
      return TradeMsg::kTypeId;
    }
    case Schema::Tbbo: {
      return TbboMsg::kTypeId;
    }
    case Schema::Ohlcv1D:
    case Schema::Ohlcv1H:
    case Schema::Ohlcv1M:
    case Schema::Ohlcv1S: {
      return OhlcvMsg::kTypeId;
    }
    case Schema::Definition: {
      return InstrumentDefMsg::kTypeId;
    }
    default: {
      throw InvalidArgumentError{
          "Record::TypeIdFromSchema", "schema",
          "unknown value '" +
              std::to_string(static_cast<std::uint16_t>(schema)) + "'"};
    }
  }
}

using databento::InstrumentDefMsg;

bool databento::operator==(const InstrumentDefMsg& lhs,
                           const InstrumentDefMsg& rhs) {
  return lhs.hd == rhs.hd && lhs.ts_recv == rhs.ts_recv &&
         lhs.min_price_increment == rhs.min_price_increment &&
         lhs.display_factor == rhs.display_factor &&
         lhs.expiration == rhs.expiration && lhs.activation == rhs.activation &&
         lhs.high_limit_price == rhs.high_limit_price &&
         lhs.low_limit_price == rhs.low_limit_price &&
         lhs.max_price_variation == rhs.max_price_variation &&
         lhs.trading_reference_price == rhs.trading_reference_price &&
         lhs.unit_of_measure_qty == rhs.unit_of_measure_qty &&
         lhs.min_price_increment_amount == rhs.min_price_increment_amount &&
         lhs.price_ratio == rhs.price_ratio &&
         lhs.inst_attrib_value == rhs.inst_attrib_value &&
         lhs.underlying_id == rhs.underlying_id &&
         lhs.cleared_volume == rhs.cleared_volume &&
         lhs.market_depth_implied == rhs.market_depth_implied &&
         lhs.market_depth == rhs.market_depth &&
         lhs.market_segment_id == rhs.market_segment_id &&
         lhs.max_trade_vol == rhs.max_trade_vol &&
         lhs.min_lot_size == rhs.min_lot_size &&
         lhs.min_lot_size_block == rhs.min_lot_size_block &&
         lhs.min_lot_size_round_lot == rhs.min_lot_size_round_lot &&
         lhs.min_trade_vol == rhs.min_trade_vol &&
         lhs.open_interest_qty == rhs.open_interest_qty &&
         lhs.contract_multiplier == rhs.contract_multiplier &&
         lhs.decay_quantity == rhs.decay_quantity &&
         lhs.original_contract_size == rhs.original_contract_size &&
         lhs.related_security_id == rhs.related_security_id &&
         lhs.trading_reference_date == rhs.trading_reference_date &&
         lhs.appl_id == rhs.appl_id && lhs.maturity_year == rhs.maturity_year &&
         lhs.decay_start_date == rhs.decay_start_date &&
         lhs.channel_id == rhs.channel_id && lhs.currency == rhs.currency &&
         lhs.settl_currency == rhs.settl_currency &&
         lhs.secsubtype == rhs.secsubtype && lhs.symbol == rhs.symbol &&
         lhs.group == rhs.group && lhs.exchange == rhs.exchange &&
         lhs.asset == rhs.asset && lhs.cfi == rhs.cfi &&
         lhs.security_type == rhs.security_type &&
         lhs.unit_of_measure == rhs.unit_of_measure &&
         lhs.underlying == rhs.underlying && lhs.related == rhs.related &&
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

namespace databento {
namespace detail {
template <>
std::string ToString(const Mbp1Msg& mbp_msg) {
  return MakeString(mbp_msg);
}
template <>
std::string ToString(const Mbp10Msg& mbp_msg) {
  return MakeString(mbp_msg);
}
template <>
std::ostream& operator<<(std::ostream& stream, const Mbp1Msg& mbp_msg) {
  return StreamOpBuilder{stream}
      .SetTypeName("Mbp1Msg")
      .SetSpacer("\n    ")
      .Build()
      .AddField("hd", mbp_msg.hd)
      .AddField("price", mbp_msg.price)
      .AddField("size", mbp_msg.size)
      .AddField("action", mbp_msg.action)
      .AddField("side", mbp_msg.side)
      .AddField("flags", mbp_msg.flags)
      .AddField("depth", mbp_msg.depth)
      .AddField("ts_recv", mbp_msg.ts_recv)
      .AddField("ts_in_delta", mbp_msg.ts_in_delta)
      .AddField("sequence", mbp_msg.sequence)
      .AddField("booklevel", std::get<0>(mbp_msg.booklevel))
      .Finish();
}
template <>
std::ostream& operator<<(std::ostream& stream, const Mbp10Msg& mbp_msg) {
  std::ostringstream booklevels;
  auto booklevel_helper =
      StreamOpBuilder{booklevels}.SetSpacer("\n    ").SetIndent("    ").Build();
  for (const auto& booklevel : mbp_msg.booklevel) {
    booklevel_helper.AddItem(booklevel);
  }
  return StreamOpBuilder{stream}
      .SetTypeName("Mbp10Msg")
      .SetSpacer("\n    ")
      .Build()
      .AddField("hd", mbp_msg.hd)
      .AddField("price", mbp_msg.price)
      .AddField("size", mbp_msg.size)
      .AddField("action", mbp_msg.action)
      .AddField("side", mbp_msg.side)
      .AddField("flags", mbp_msg.flags)
      .AddField("depth", mbp_msg.depth)
      .AddField("ts_recv", mbp_msg.ts_recv)
      .AddField("ts_in_delta", mbp_msg.ts_in_delta)
      .AddField("sequence", mbp_msg.sequence)
      .AddField("booklevel",
                static_cast<std::ostringstream&>(booklevel_helper.Finish()))
      .Finish();
}
}  // namespace detail

std::string ToString(const RecordHeader& header) { return MakeString(header); }
std::ostream& operator<<(std::ostream& stream, const RecordHeader& header) {
  return StreamOpBuilder{stream}
      .SetSpacer(" ")
      .SetTypeName("RecordHeader")
      .Build()
      .AddField("length", header.length)
      .AddField("rtype", header.rtype)
      .AddField("publisher_id", header.publisher_id)
      .AddField("product_id", header.product_id)
      .AddField("ts_event", header.ts_event)
      .Finish();
}
std::string ToString(const MboMsg& mbo_msg) { return MakeString(mbo_msg); }
std::ostream& operator<<(std::ostream& stream, const MboMsg& mbo_msg) {
  return StreamOpBuilder{stream}
      .SetSpacer("\n    ")
      .SetTypeName("MboMsg")
      .Build()
      .AddField("hd", mbo_msg.hd)
      .AddField("order_id", mbo_msg.order_id)
      .AddField("price", mbo_msg.price)
      .AddField("size", mbo_msg.size)
      .AddField("flags", mbo_msg.flags)
      .AddField("channel_id", mbo_msg.channel_id)
      .AddField("action", mbo_msg.action)
      .AddField("side", mbo_msg.side)
      .AddField("ts_recv", mbo_msg.ts_recv)
      .AddField("ts_in_delta", mbo_msg.ts_in_delta)
      .AddField("sequence", mbo_msg.sequence)
      .Finish();
}
std::string ToString(const BidAskPair& ba_pair) { return MakeString(ba_pair); }
std::ostream& operator<<(std::ostream& stream, const BidAskPair& ba_pair) {
  return StreamOpBuilder{stream}
      .SetSpacer(" ")
      .SetTypeName("BidAskPair")
      .Build()
      .AddField("bid_px", ba_pair.bid_px)
      .AddField("ask_px", ba_pair.ask_px)
      .AddField("bid_sz", ba_pair.bid_sz)
      .AddField("ask_sz", ba_pair.ask_sz)
      .AddField("bid_ct", ba_pair.bid_ct)
      .AddField("ask_ct", ba_pair.ask_ct)
      .Finish();
}
std::string ToString(const TradeMsg& trade_msg) {
  return MakeString(trade_msg);
}
std::ostream& operator<<(std::ostream& stream, const TradeMsg& trade_msg) {
  return StreamOpBuilder{stream}
      .SetSpacer("\n    ")
      .SetTypeName("TradeMsg")
      .Build()
      .AddField("hd", trade_msg.hd)
      .AddField("price", trade_msg.price)
      .AddField("size", trade_msg.size)
      .AddField("action", trade_msg.action)
      .AddField("side", trade_msg.side)
      .AddField("flags", trade_msg.flags)
      .AddField("depth", trade_msg.depth)
      .AddField("ts_recv", trade_msg.ts_recv)
      .AddField("ts_in_delta", trade_msg.ts_in_delta)
      .AddField("sequence", trade_msg.sequence)
      .Finish();
}
std::string ToString(const OhlcvMsg& ohlcv_msg) {
  return MakeString(ohlcv_msg);
}
std::ostream& operator<<(std::ostream& stream, const OhlcvMsg& ohlcv_msg) {
  return StreamOpBuilder{stream}
      .SetSpacer("\n    ")
      .SetTypeName("OhlcvMsg")
      .Build()
      .AddField("hd", ohlcv_msg.hd)
      .AddField("open", ohlcv_msg.open)
      .AddField("high", ohlcv_msg.high)
      .AddField("low", ohlcv_msg.low)
      .AddField("close", ohlcv_msg.close)
      .AddField("volume", ohlcv_msg.volume)
      .Finish();
}
std::string ToString(const InstrumentDefMsg& instr_def_msg) {
  return MakeString(instr_def_msg);
}
std::ostream& operator<<(std::ostream& stream,
                         const InstrumentDefMsg& instr_def_msg) {
  return StreamOpBuilder{stream}
      .SetSpacer("\n    ")
      .SetTypeName("InstrumentDefMsg")
      .Build()
      .AddField("hd", instr_def_msg.hd)
      .AddField("ts_recv", instr_def_msg.ts_recv)
      .AddField("min_price_increment", instr_def_msg.min_price_increment)
      .AddField("display_factor", instr_def_msg.display_factor)
      .AddField("expiration", instr_def_msg.expiration)
      .AddField("activation", instr_def_msg.activation)
      .AddField("high_limit_price", instr_def_msg.high_limit_price)
      .AddField("low_limit_price", instr_def_msg.low_limit_price)
      .AddField("max_price_variation", instr_def_msg.max_price_variation)
      .AddField("trading_reference_price",
                instr_def_msg.trading_reference_price)
      .AddField("unit_of_measure_qty", instr_def_msg.unit_of_measure_qty)
      .AddField("min_price_increment_amount",
                instr_def_msg.min_price_increment_amount)
      .AddField("price_ratio", instr_def_msg.price_ratio)
      .AddField("inst_attrib_value", instr_def_msg.inst_attrib_value)
      .AddField("underlying_id", instr_def_msg.underlying_id)
      .AddField("cleared_volume", instr_def_msg.cleared_volume)
      .AddField("market_depth_implied", instr_def_msg.market_depth_implied)
      .AddField("market_depth", instr_def_msg.market_depth)
      .AddField("market_segment_id", instr_def_msg.market_segment_id)
      .AddField("max_trade_vol", instr_def_msg.max_trade_vol)
      .AddField("min_lot_size", instr_def_msg.min_lot_size)
      .AddField("min_lot_size_block", instr_def_msg.min_lot_size_block)
      .AddField("min_lot_size_round_lot", instr_def_msg.min_lot_size_round_lot)
      .AddField("min_trade_vol", instr_def_msg.min_trade_vol)
      .AddField("open_interest_qty", instr_def_msg.open_interest_qty)
      .AddField("contract_multiplier", instr_def_msg.contract_multiplier)
      .AddField("decay_quantity", instr_def_msg.decay_quantity)
      .AddField("original_contract_size", instr_def_msg.original_contract_size)
      .AddField("related_security_id", instr_def_msg.related_security_id)
      .AddField("trading_reference_date", instr_def_msg.trading_reference_date)
      .AddField("appl_id", instr_def_msg.appl_id)
      .AddField("maturity_year", instr_def_msg.maturity_year)
      .AddField("decay_start_date", instr_def_msg.decay_start_date)
      .AddField("channel_id", instr_def_msg.channel_id)
      .AddField("currency", instr_def_msg.currency)
      .AddField("settl_currency", instr_def_msg.settl_currency)
      .AddField("secsubtype", instr_def_msg.secsubtype)
      .AddField("symbol", instr_def_msg.symbol)
      .AddField("group", instr_def_msg.group)
      .AddField("exchange", instr_def_msg.exchange)
      .AddField("asset", instr_def_msg.asset)
      .AddField("cfi", instr_def_msg.cfi)
      .AddField("security_type", instr_def_msg.security_type)
      .AddField("unit_of_measure", instr_def_msg.unit_of_measure)
      .AddField("underlying", instr_def_msg.underlying)
      .AddField("related", instr_def_msg.related)
      .AddField("match_algorithm", instr_def_msg.match_algorithm)
      .AddField("md_security_trading_status",
                instr_def_msg.md_security_trading_status)
      .AddField("main_fraction", instr_def_msg.main_fraction)
      .AddField("price_display_format", instr_def_msg.price_display_format)
      .AddField("settl_price_type", instr_def_msg.settl_price_type)
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
      .Finish();
}
}  // namespace databento
