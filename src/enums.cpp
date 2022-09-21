#include "databento/enums.hpp"

#include <cstdint>
#include <stdexcept>
#include <string>

namespace databento {
const char* UrlFromGateway(HistoricalGateway gateway) {
  switch (gateway) {
    case HistoricalGateway::Nearest:
    case HistoricalGateway::Bo1: {
      return "https://hist.databento.com";
    }
    default: {
      throw std::invalid_argument{
          "Invalid HistoricalGateway " +
          std::to_string(static_cast<std::uint8_t>(gateway))};
    }
  }
}
}  // namespace databento
