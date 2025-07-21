#include "databento/record.hpp"

#include <string>

#include "databento/enums.hpp"
#include "databento/exceptions.hpp"  // InvalidArgumentError
#include "databento/pretty.hpp"      // Px
#include "stream_op_helper.hpp"

using databento::Record;
using databento::RecordHeader;

std::size_t RecordHeader::Size() const {
  return static_cast<size_t>(length) * kLengthMultiplier;
}

std::size_t Record::Size() const { return record_->Size(); }

std::size_t Record::SizeOfSchema(const Schema schema) {
  switch (schema) {
    case Schema::Mbo: {
      return sizeof(MboMsg);
    }
    case Schema::Mbp1: {
      return sizeof(Mbp1Msg);
    }
    case Schema::Mbp10: {
      return sizeof(Mbp10Msg);
    }
    case Schema::Trades: {
      return sizeof(TradeMsg);
    }
    case Schema::Tbbo: {
      return sizeof(Mbp1Msg);
    }
    case Schema::Ohlcv1S:
      [[fallthrough]];
    case Schema::Ohlcv1M:
      [[fallthrough]];
    case Schema::Ohlcv1H:
      [[fallthrough]];
    case Schema::Ohlcv1D: {
      return sizeof(OhlcvMsg);
    }
    case Schema::Definition: {
      return sizeof(InstrumentDefMsg);
    }
    case Schema::Statistics: {
      return sizeof(StatMsg);
    }
    case Schema::Imbalance: {
      return sizeof(ImbalanceMsg);
    }
    default: {
      throw InvalidArgumentError{
          "Record::SizeOfSchema", "schema",
          "unknown value '" + std::to_string(static_cast<std::uint16_t>(schema)) + "'"};
    }
  }
}

databento::RType Record::RTypeFromSchema(const Schema schema) {
  switch (schema) {
    case Schema::Mbo: {
      return databento::RType::Mbo;
    }
    case Schema::Mbp1: {
      return databento::RType::Mbp1;
    }
    case Schema::Mbp10: {
      return databento::RType::Mbp10;
    }
    case Schema::Trades: {
      return databento::RType::Mbp0;
    }
    case Schema::Tbbo: {
      return databento::RType::Mbp1;
    }
    case Schema::Ohlcv1S: {
      return databento::RType::Ohlcv1S;
    }
    case Schema::Ohlcv1M: {
      return databento::RType::Ohlcv1M;
    }
    case Schema::Ohlcv1H: {
      return databento::RType::Ohlcv1H;
    }
    case Schema::Ohlcv1D: {
      return databento::RType::Ohlcv1D;
    }
    case Schema::Definition: {
      return databento::RType::InstrumentDef;
    }
    case Schema::Statistics: {
      return databento::RType::Statistics;
    }
    case Schema::Imbalance: {
      return databento::RType::Imbalance;
    }
    default: {
      throw InvalidArgumentError{
          "Record::RTypeFromSchema", "schema",
          "unknown value '" + std::to_string(static_cast<std::uint16_t>(schema)) + "'"};
    }
  }
}

using databento::InstrumentDefMsg;

bool databento::operator==(const InstrumentDefMsg& lhs, const InstrumentDefMsg& rhs) {
  return lhs.hd == rhs.hd && lhs.ts_recv == rhs.ts_recv &&
         lhs.min_price_increment == rhs.min_price_increment &&
         lhs.display_factor == rhs.display_factor && lhs.expiration == rhs.expiration &&
         lhs.activation == rhs.activation &&
         lhs.high_limit_price == rhs.high_limit_price &&
         lhs.low_limit_price == rhs.low_limit_price &&
         lhs.max_price_variation == rhs.max_price_variation &&
         lhs.unit_of_measure_qty == rhs.unit_of_measure_qty &&
         lhs.min_price_increment_amount == rhs.min_price_increment_amount &&
         lhs.price_ratio == rhs.price_ratio && lhs.strike_price == rhs.strike_price &&
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
         lhs.leg_underlying_id == rhs.leg_underlying_id && lhs.appl_id == rhs.appl_id &&
         lhs.maturity_year == rhs.maturity_year &&
         lhs.decay_start_date == rhs.decay_start_date &&
         lhs.channel_id == rhs.channel_id && lhs.leg_count == rhs.leg_count &&
         lhs.leg_index == rhs.leg_index && lhs.currency == rhs.currency &&
         lhs.settl_currency == rhs.settl_currency && lhs.secsubtype == rhs.secsubtype &&
         lhs.raw_symbol == rhs.raw_symbol && lhs.group == rhs.group &&
         lhs.exchange == rhs.exchange && lhs.asset == rhs.asset && lhs.cfi == rhs.cfi &&
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

using databento::ImbalanceMsg;

bool databento::operator==(const ImbalanceMsg& lhs, const ImbalanceMsg& rhs) {
  return lhs.hd == rhs.hd && lhs.ts_recv == rhs.ts_recv &&
         lhs.ref_price == rhs.ref_price && lhs.auction_time == rhs.auction_time &&
         lhs.cont_book_clr_price == rhs.cont_book_clr_price &&
         lhs.auct_interest_clr_price == rhs.auct_interest_clr_price &&
         lhs.ssr_filling_price == rhs.ssr_filling_price &&
         lhs.ind_match_price == rhs.ind_match_price &&
         lhs.upper_collar == rhs.upper_collar && lhs.lower_collar == rhs.lower_collar &&
         lhs.paired_qty == rhs.paired_qty &&
         lhs.total_imbalance_qty == rhs.total_imbalance_qty &&
         lhs.market_imbalance_qty == rhs.market_imbalance_qty &&
         lhs.unpaired_qty == rhs.unpaired_qty && lhs.auction_type == rhs.auction_type &&
         lhs.side == rhs.side && lhs.auction_status == rhs.auction_status &&
         lhs.freeze_status == rhs.freeze_status &&
         lhs.num_extensions == rhs.num_extensions &&
         lhs.unpaired_side == rhs.unpaired_side &&
         lhs.significant_imbalance == rhs.significant_imbalance;
}

namespace databento {
std::string ToString(const Record& record) { return MakeString(record); }
std::ostream& operator<<(std::ostream& stream, const Record& record) {
  return StreamOpBuilder{stream}
      .SetSpacer(" ")
      .SetTypeName("Record")
      .Build()
      .AddField("ptr", record.Header())
      .Finish();
}
std::string ToString(const RecordHeader& header) { return MakeString(header); }
std::ostream& operator<<(std::ostream& stream, const RecordHeader& header) {
  return StreamOpBuilder{stream}
      .SetSpacer(" ")
      .SetTypeName("RecordHeader")
      .Build()
      .AddField("length", header.length)
      .AddField("rtype", header.rtype)
      .AddField("publisher_id", header.publisher_id)
      .AddField("instrument_id", header.instrument_id)
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
      .AddField("price", pretty::Px{mbo_msg.price})
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

std::string ToString(const BidAskPair& bid_ask_pair) {
  return MakeString(bid_ask_pair);
}
std::ostream& operator<<(std::ostream& stream, const BidAskPair& bid_ask_pair) {
  return StreamOpBuilder{stream}
      .SetSpacer(" ")
      .SetTypeName("BidAskPair")
      .Build()
      .AddField("bid_px", pretty::Px{bid_ask_pair.bid_px})
      .AddField("ask_px", pretty::Px{bid_ask_pair.ask_px})
      .AddField("bid_sz", bid_ask_pair.bid_sz)
      .AddField("ask_sz", bid_ask_pair.ask_sz)
      .AddField("bid_ct", bid_ask_pair.bid_ct)
      .AddField("ask_ct", bid_ask_pair.ask_ct)
      .Finish();
}

std::string ToString(const ConsolidatedBidAskPair& consolidated_bid_ask_pair) {
  return MakeString(consolidated_bid_ask_pair);
}
std::ostream& operator<<(std::ostream& stream,
                         const ConsolidatedBidAskPair& consolidated_bid_ask_pair) {
  return StreamOpBuilder{stream}
      .SetSpacer(" ")
      .SetTypeName("ConsolidatedBidAskPair")
      .Build()
      .AddField("bid_px", pretty::Px{consolidated_bid_ask_pair.bid_px})
      .AddField("ask_px", pretty::Px{consolidated_bid_ask_pair.ask_px})
      .AddField("bid_sz", consolidated_bid_ask_pair.bid_sz)
      .AddField("ask_sz", consolidated_bid_ask_pair.ask_sz)
      .AddField("bid_pb", consolidated_bid_ask_pair.bid_pb)
      .AddField("ask_pb", consolidated_bid_ask_pair.ask_pb)
      .Finish();
}

std::string ToString(const TradeMsg& trade_msg) { return MakeString(trade_msg); }
std::ostream& operator<<(std::ostream& stream, const TradeMsg& trade_msg) {
  return StreamOpBuilder{stream}
      .SetSpacer("\n    ")
      .SetTypeName("TradeMsg")
      .Build()
      .AddField("hd", trade_msg.hd)
      .AddField("price", pretty::Px{trade_msg.price})
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

std::string ToString(const Mbp1Msg& mbp1_msg) { return MakeString(mbp1_msg); }
std::ostream& operator<<(std::ostream& stream, const Mbp1Msg& mbp1_msg) {
  return StreamOpBuilder{stream}
      .SetSpacer("\n    ")
      .SetTypeName("Mbp1Msg")
      .Build()
      .AddField("hd", mbp1_msg.hd)
      .AddField("price", pretty::Px{mbp1_msg.price})
      .AddField("size", mbp1_msg.size)
      .AddField("action", mbp1_msg.action)
      .AddField("side", mbp1_msg.side)
      .AddField("flags", mbp1_msg.flags)
      .AddField("depth", mbp1_msg.depth)
      .AddField("ts_recv", mbp1_msg.ts_recv)
      .AddField("ts_in_delta", mbp1_msg.ts_in_delta)
      .AddField("sequence", mbp1_msg.sequence)

      .AddField("levels", std::get<0>(mbp1_msg.levels))
      .Finish();
}

std::string ToString(const Mbp10Msg& mbp10_msg) { return MakeString(mbp10_msg); }
std::ostream& operator<<(std::ostream& stream, const Mbp10Msg& mbp10_msg) {
  std::ostringstream sub_ss;
  auto helper = StreamOpBuilder{sub_ss}.SetSpacer("\n    ").SetIndent("    ").Build();
  for (const auto& level : mbp10_msg.levels) {
    helper.AddItem(level);
  }

  return StreamOpBuilder{stream}
      .SetSpacer("\n    ")
      .SetTypeName("Mbp10Msg")
      .Build()
      .AddField("hd", mbp10_msg.hd)
      .AddField("price", pretty::Px{mbp10_msg.price})
      .AddField("size", mbp10_msg.size)
      .AddField("action", mbp10_msg.action)
      .AddField("side", mbp10_msg.side)
      .AddField("flags", mbp10_msg.flags)
      .AddField("depth", mbp10_msg.depth)
      .AddField("ts_recv", mbp10_msg.ts_recv)
      .AddField("ts_in_delta", mbp10_msg.ts_in_delta)
      .AddField("sequence", mbp10_msg.sequence)
      .AddField("levels", static_cast<std::ostringstream&>(helper.Finish()))
      .Finish();
}

std::string ToString(const BboMsg& bbo_msg) { return MakeString(bbo_msg); }
std::ostream& operator<<(std::ostream& stream, const BboMsg& bbo_msg) {
  return StreamOpBuilder{stream}
      .SetSpacer("\n    ")
      .SetTypeName("BboMsg")
      .Build()
      .AddField("hd", bbo_msg.hd)
      .AddField("price", pretty::Px{bbo_msg.price})
      .AddField("size", bbo_msg.size)
      .AddField("side", bbo_msg.side)
      .AddField("flags", bbo_msg.flags)
      .AddField("ts_recv", bbo_msg.ts_recv)
      .AddField("sequence", bbo_msg.sequence)

      .AddField("levels", std::get<0>(bbo_msg.levels))
      .Finish();
}

std::string ToString(const Cmbp1Msg& cmbp1_msg) { return MakeString(cmbp1_msg); }
std::ostream& operator<<(std::ostream& stream, const Cmbp1Msg& cmbp1_msg) {
  return StreamOpBuilder{stream}
      .SetSpacer("\n    ")
      .SetTypeName("Cmbp1Msg")
      .Build()
      .AddField("hd", cmbp1_msg.hd)
      .AddField("price", pretty::Px{cmbp1_msg.price})
      .AddField("size", cmbp1_msg.size)
      .AddField("action", cmbp1_msg.action)
      .AddField("side", cmbp1_msg.side)
      .AddField("flags", cmbp1_msg.flags)
      .AddField("ts_recv", cmbp1_msg.ts_recv)
      .AddField("ts_in_delta", cmbp1_msg.ts_in_delta)

      .AddField("levels", std::get<0>(cmbp1_msg.levels))
      .Finish();
}

std::string ToString(const CbboMsg& cbbo_msg) { return MakeString(cbbo_msg); }
std::ostream& operator<<(std::ostream& stream, const CbboMsg& cbbo_msg) {
  return StreamOpBuilder{stream}
      .SetSpacer("\n    ")
      .SetTypeName("CbboMsg")
      .Build()
      .AddField("hd", cbbo_msg.hd)
      .AddField("price", pretty::Px{cbbo_msg.price})
      .AddField("size", cbbo_msg.size)
      .AddField("side", cbbo_msg.side)
      .AddField("flags", cbbo_msg.flags)
      .AddField("ts_recv", cbbo_msg.ts_recv)

      .AddField("levels", std::get<0>(cbbo_msg.levels))
      .Finish();
}

std::string ToString(const OhlcvMsg& ohlcv_msg) { return MakeString(ohlcv_msg); }
std::ostream& operator<<(std::ostream& stream, const OhlcvMsg& ohlcv_msg) {
  return StreamOpBuilder{stream}
      .SetSpacer("\n    ")
      .SetTypeName("OhlcvMsg")
      .Build()
      .AddField("hd", ohlcv_msg.hd)
      .AddField("open", pretty::Px{ohlcv_msg.open})
      .AddField("high", pretty::Px{ohlcv_msg.high})
      .AddField("low", pretty::Px{ohlcv_msg.low})
      .AddField("close", pretty::Px{ohlcv_msg.close})
      .AddField("volume", ohlcv_msg.volume)
      .Finish();
}

std::string ToString(const StatusMsg& status_msg) { return MakeString(status_msg); }
std::ostream& operator<<(std::ostream& stream, const StatusMsg& status_msg) {
  return StreamOpBuilder{stream}
      .SetSpacer("\n    ")
      .SetTypeName("StatusMsg")
      .Build()
      .AddField("hd", status_msg.hd)
      .AddField("ts_recv", status_msg.ts_recv)
      .AddField("action", status_msg.action)
      .AddField("reason", status_msg.reason)
      .AddField("trading_event", status_msg.trading_event)
      .AddField("is_trading", status_msg.is_trading)
      .AddField("is_quoting", status_msg.is_quoting)
      .AddField("is_short_sell_restricted", status_msg.is_short_sell_restricted)
      .Finish();
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
      .AddField("unit_of_measure_qty",
                pretty::Px{instrument_def_msg.unit_of_measure_qty})
      .AddField("min_price_increment_amount",
                pretty::Px{instrument_def_msg.min_price_increment_amount})
      .AddField("price_ratio", pretty::Px{instrument_def_msg.price_ratio})
      .AddField("strike_price", pretty::Px{instrument_def_msg.strike_price})
      .AddField("raw_instrument_id", instrument_def_msg.raw_instrument_id)
      .AddField("leg_price", pretty::Px{instrument_def_msg.leg_price})
      .AddField("leg_delta", pretty::Px{instrument_def_msg.leg_delta})
      .AddField("inst_attrib_value", instrument_def_msg.inst_attrib_value)
      .AddField("underlying_id", instrument_def_msg.underlying_id)
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
      .AddField("leg_instrument_id", instrument_def_msg.leg_instrument_id)
      .AddField("leg_ratio_price_numerator",
                instrument_def_msg.leg_ratio_price_numerator)
      .AddField("leg_ratio_price_denominator",
                instrument_def_msg.leg_ratio_price_denominator)
      .AddField("leg_ratio_qty_numerator", instrument_def_msg.leg_ratio_qty_numerator)
      .AddField("leg_ratio_qty_denominator",
                instrument_def_msg.leg_ratio_qty_denominator)
      .AddField("leg_underlying_id", instrument_def_msg.leg_underlying_id)
      .AddField("appl_id", instrument_def_msg.appl_id)
      .AddField("maturity_year", instrument_def_msg.maturity_year)
      .AddField("decay_start_date", instrument_def_msg.decay_start_date)
      .AddField("channel_id", instrument_def_msg.channel_id)
      .AddField("leg_count", instrument_def_msg.leg_count)
      .AddField("leg_index", instrument_def_msg.leg_index)
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
      .AddField("leg_raw_symbol", instrument_def_msg.leg_raw_symbol)
      .AddField("instrument_class", instrument_def_msg.instrument_class)
      .AddField("match_algorithm", instrument_def_msg.match_algorithm)
      .AddField("main_fraction", instrument_def_msg.main_fraction)
      .AddField("price_display_format", instrument_def_msg.price_display_format)
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
      .AddField("leg_instrument_class", instrument_def_msg.leg_instrument_class)
      .AddField("leg_side", instrument_def_msg.leg_side)
      .Finish();
}

std::string ToString(const ImbalanceMsg& imbalance_msg) {
  return MakeString(imbalance_msg);
}
std::ostream& operator<<(std::ostream& stream, const ImbalanceMsg& imbalance_msg) {
  return StreamOpBuilder{stream}
      .SetSpacer("\n    ")
      .SetTypeName("ImbalanceMsg")
      .Build()
      .AddField("hd", imbalance_msg.hd)
      .AddField("ts_recv", imbalance_msg.ts_recv)
      .AddField("ref_price", pretty::Px{imbalance_msg.ref_price})
      .AddField("auction_time", imbalance_msg.auction_time)
      .AddField("cont_book_clr_price", pretty::Px{imbalance_msg.cont_book_clr_price})
      .AddField("auct_interest_clr_price",
                pretty::Px{imbalance_msg.auct_interest_clr_price})
      .AddField("ssr_filling_price", pretty::Px{imbalance_msg.ssr_filling_price})
      .AddField("ind_match_price", pretty::Px{imbalance_msg.ind_match_price})
      .AddField("upper_collar", pretty::Px{imbalance_msg.upper_collar})
      .AddField("lower_collar", pretty::Px{imbalance_msg.lower_collar})
      .AddField("paired_qty", imbalance_msg.paired_qty)
      .AddField("total_imbalance_qty", imbalance_msg.total_imbalance_qty)
      .AddField("market_imbalance_qty", imbalance_msg.market_imbalance_qty)
      .AddField("unpaired_qty", imbalance_msg.unpaired_qty)
      .AddField("auction_type", imbalance_msg.auction_type)
      .AddField("side", imbalance_msg.side)
      .AddField("auction_status", imbalance_msg.auction_status)
      .AddField("freeze_status", imbalance_msg.freeze_status)
      .AddField("num_extensions", imbalance_msg.num_extensions)
      .AddField("unpaired_side", imbalance_msg.unpaired_side)
      .AddField("significant_imbalance", imbalance_msg.significant_imbalance)
      .Finish();
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

std::string ToString(const ErrorMsg& error_msg) { return MakeString(error_msg); }
std::ostream& operator<<(std::ostream& stream, const ErrorMsg& error_msg) {
  return StreamOpBuilder{stream}
      .SetSpacer("\n    ")
      .SetTypeName("ErrorMsg")
      .Build()
      .AddField("hd", error_msg.hd)
      .AddField("err", error_msg.err)
      .AddField("code", error_msg.code)
      .AddField("is_last", error_msg.is_last)
      .Finish();
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
      .AddField("stype_in", symbol_mapping_msg.stype_in)
      .AddField("stype_in_symbol", symbol_mapping_msg.stype_in_symbol)
      .AddField("stype_out", symbol_mapping_msg.stype_out)
      .AddField("stype_out_symbol", symbol_mapping_msg.stype_out_symbol)
      .AddField("start_ts", symbol_mapping_msg.start_ts)
      .AddField("end_ts", symbol_mapping_msg.end_ts)
      .Finish();
}

std::string ToString(const SystemMsg& system_msg) { return MakeString(system_msg); }
std::ostream& operator<<(std::ostream& stream, const SystemMsg& system_msg) {
  return StreamOpBuilder{stream}
      .SetSpacer("\n    ")
      .SetTypeName("SystemMsg")
      .Build()
      .AddField("hd", system_msg.hd)
      .AddField("msg", system_msg.msg)
      .AddField("code", system_msg.code)
      .Finish();
}

}  // namespace databento
