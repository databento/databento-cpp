#include "databento/record.hpp"

#include <stdexcept>
#include <string>

#include "databento/enums.hpp"

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
      throw std::logic_error{"Invalid rtype_ " + std::to_string(rtype)};
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
      throw std::invalid_argument{
          "Unknown schema " +
          std::to_string(static_cast<std::uint16_t>(schema))};
    }
  }
}
