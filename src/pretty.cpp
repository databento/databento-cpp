#include "databento/pretty.hpp"

#include <array>
#include <cstdint>
#include <iomanip>
#include <sstream>

#include "databento/datetime.hpp"

namespace databento::pretty {
std::ostream& operator<<(std::ostream& stream, Px px) {
  constexpr std::array<std::int64_t, 6> kDivisors = {0,         100'000'000, 10'000'000,
                                                     1'000'000, 100'000,     10'000};
  if (px.IsUndefined()) {
    stream << "UNDEF_PRICE";
    return stream;
  }
  const bool is_negative = (px.val < 0);
  const int64_t px_abs = is_negative ? -px.val : px.val;
  const int64_t price_integer = px_abs / kFixedPriceScale;
  const int64_t price_fraction = px_abs % kFixedPriceScale;
  std::ostringstream ss;
  if (is_negative) {
    ss << '-';
  }
  const auto orig_precision = static_cast<int>(stream.precision());
  if (orig_precision == 0) {
    ss << price_integer;
  } else {
    // Don't support precision 6-8 (inclusive). 6 is the default and there's no
    // way to disambiguate between explicitly set 6 and the default value,
    // however by default we want to print all 9 digits
    const auto precision = orig_precision < 6 ? orig_precision : 9;
    ss << price_integer << '.' << std::setw(precision) << std::setfill('0')
       << (precision == 9
               ? price_fraction
               : price_fraction / kDivisors[static_cast<std::size_t>(precision)]);
  }
  stream << ss.str();
  return stream;
}

std::ostream& operator<<(std::ostream& stream, Ts ts) {
  stream << ToIso8601(ts.val);
  return stream;
}
}  // namespace databento::pretty
