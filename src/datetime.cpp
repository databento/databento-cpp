#include "databento/datetime.hpp"

#include <iomanip>
#include <sstream>

namespace databento {
std::string ToString(EpochNanos epoch_nanos) {
  return std::to_string(epoch_nanos.time_since_epoch().count());
}

std::string DateFromIso8601Int(std::uint32_t date_int) {
  const auto year = date_int / 10000;
  const auto remaining = date_int % 10000;
  const auto month = remaining / 100;
  const auto day = remaining % 100;
  std::ostringstream out_ss;
  out_ss << year << '-' << std::setfill('0') << std::setw(2) << month << '-'
         << std::setfill('0') << std::setw(2) << day;
  return out_ss.str();
}
}  // namespace databento
