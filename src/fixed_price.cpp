#include "databento/fixed_price.hpp"

#include <iomanip>

namespace databento {
std::ostream& operator<<(std::ostream& stream, FixPx fix_px) {
  if (fix_px.IsUndefined()) {
    stream << "kUndefPrice";
    return stream;
  }
  const bool price_neg = (fix_px.val < 0);
  const int64_t fixed_price_abs = price_neg ? -fix_px.val : fix_px.val;
  const int64_t price_integer = fixed_price_abs / kFixedPriceScale;
  const int64_t price_fraction = fixed_price_abs % kFixedPriceScale;
  if (price_neg) {
    stream << '-';
  }
  stream << price_integer << '.' << std::setw(9) << std::setfill('0')
         << price_fraction;
  return stream;
}
}  // namespace databento
