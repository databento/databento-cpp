#pragma once

#include "databento/constants.hpp"  // kRecordHeaderLengthMultiplier
#include "databento/datetime.hpp"   // UnixNanos
#include "databento/enums.hpp"      // RType

namespace databento {
// Record wrapper to read records with their live gateway send
// timestamp (ts_out).
template <typename R>
struct WithTsOut {
  static bool HasRType(RType rtype) { return R::HasRType(rtype); }

  WithTsOut(R r, UnixNanos ts) : rec{r}, ts_out{ts} {
    // Adjust length for `ts_out`
    this->rec.hd.length = sizeof(*this) / kRecordHeaderLengthMultiplier;
  }

  // The base record.
  R rec;
  // The end timestamp from the Databento live gateway.
  UnixNanos ts_out;
};

template <typename R>
inline bool operator==(const WithTsOut<R>& lhs, const WithTsOut<R>& rhs) {
  return lhs.rec == rhs.rec && lhs.ts_out == rhs.ts_out;
}
template <typename R>
inline bool operator!=(const WithTsOut<R>& lhs, const WithTsOut<R>& rhs) {
  return !(lhs == rhs);
}
}  // namespace databento
