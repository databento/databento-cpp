#include "databento/record.hpp"

#include <string>

#include "databento/enums.hpp"
#include "databento/exceptions.hpp"  // InvalidArgumentError
#include "stream_op_helper.hpp"

using databento::Record;

std::size_t Record::size() const { return Record::SizeOfType(record_->rtype); }

std::size_t Record::SizeOfType(const std::uint8_t rtype) {
  switch (rtype) {
    case TickMsg::kTypeId: {
      return sizeof(TickMsg);
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
      return TickMsg::kTypeId;
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
    default: {
      throw InvalidArgumentError{
          "Record::TypeIdFromSchema", "schema",
          "unknown value '" +
              std::to_string(static_cast<std::uint16_t>(schema)) + "'"};
    }
  }
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
std::string ToString(const TickMsg& tick_msg) { return MakeString(tick_msg); }
std::ostream& operator<<(std::ostream& stream, const TickMsg& tick_msg) {
  return StreamOpBuilder{stream}
      .SetSpacer("\n    ")
      .SetTypeName("TickMsg")
      .Build()
      .AddField("hd", tick_msg.hd)
      .AddField("order_id", tick_msg.order_id)
      .AddField("price", tick_msg.price)
      .AddField("size", tick_msg.size)
      .AddField("flags", tick_msg.flags)
      .AddField("channel_id", tick_msg.channel_id)
      .AddField("action", tick_msg.action)
      .AddField("side", tick_msg.side)
      .AddField("ts_recv", tick_msg.ts_recv)
      .AddField("ts_in_delta", tick_msg.ts_in_delta)
      .AddField("sequence", tick_msg.sequence)
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
}  // namespace databento
