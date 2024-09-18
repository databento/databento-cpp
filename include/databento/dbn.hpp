#pragma once

#include <date/date.h>

#include <cstdint>
#include <ostream>
#include <string>
#include <vector>

#include "databento/datetime.hpp"  // UnixNanos
#include "databento/enums.hpp"

namespace databento {
// Forward declare
class PitSymbolMap;
class TsSymbolMap;

struct MappingInterval {
  // The start date of the interval (inclusive).
  date::year_month_day start_date;
  // The end date of the interval (exclusive).
  date::year_month_day end_date;
  // The resolved symbol for this interval (in `stype_out`).
  std::string symbol;
};

struct SymbolMapping {
  // The `stype_in` symbol.
  std::string raw_symbol;
  // The mappings of `raw_symbol` to `stype_out` for different date ranges.
  std::vector<MappingInterval> intervals;
};

// Information about a DBN stream.
struct Metadata {
  // The DBN schema version number.
  std::uint8_t version;
  // The dataset code.
  std::string dataset;
  // Indicates the DBN stream may contain multiple record types and data
  // schemas. If true, the schema field should be ignored.
  bool has_mixed_schema;
  // The data record schema which affects the type record of present. If
  // has_mixed_schema is true, this field should be ignored.
  Schema schema;
  // The UNIX timestamp of the query start, or the first record if the file was
  // split.
  UnixNanos start;
  // The UNIX timestamp of the query end, or the last record if the file was
  // split.
  UnixNanos end;
  // The maximum number of records for the query.
  std::uint64_t limit;
  // Indicates the DBN stream may been create with a mix of stype_in. This
  // will always be the case for live data where stype_in is specified per
  // subscription.
  bool has_mixed_stype_in;
  // The input symbology type. Should be ignored if has_mixed_schemas is true.
  SType stype_in;
  // The output symbology type.
  SType stype_out;
  // Whether the records contain an appended send timestamp.
  bool ts_out;
  // The length in bytes of fixed-length symbol strings, including a null
  // terminator byte.
  std::size_t symbol_cstr_len;
  // The original query input symbols from the request.
  std::vector<std::string> symbols;
  // Symbols that did not resolve for _at least one day_ in the query time
  // range.
  std::vector<std::string> partial;
  // Symbols that did not resolve for _any_ day in the query time range.
  std::vector<std::string> not_found;
  // Symbol mappings containing a native symbol and its mapping intervals.
  std::vector<SymbolMapping> mappings;

  // Creates a symbology mapping from instrument ID to text symbol for the given
  // date.
  //
  // This method is useful when working with a historical request over a single
  // day or in other situations where you're sure the mappings don't change
  // during the time range of the request. Otherwise, `SymbolMap()` is
  // recommmended.
  PitSymbolMap CreateSymbolMapForDate(date::year_month_day date) const;
  // Creates a symbology mapping from instrument ID and date to text symbol.
  TsSymbolMap CreateSymbolMap() const;
  // Upgrades the metadata according to `upgrade_policy` if necessary.
  void Upgrade(VersionUpgradePolicy upgrade_policy);
};

inline bool operator==(const MappingInterval& lhs, const MappingInterval& rhs) {
  return lhs.start_date == rhs.start_date && lhs.end_date == rhs.end_date &&
         lhs.symbol == rhs.symbol;
}
inline bool operator!=(const MappingInterval& lhs, const MappingInterval& rhs) {
  return !(lhs == rhs);
}

inline bool operator==(const SymbolMapping& lhs, const SymbolMapping& rhs) {
  return lhs.raw_symbol == rhs.raw_symbol && lhs.intervals == rhs.intervals;
}
inline bool operator!=(const SymbolMapping& lhs, const SymbolMapping& rhs) {
  return !(lhs == rhs);
}

inline bool operator==(const Metadata& lhs, const Metadata& rhs) {
  return lhs.version == rhs.version && lhs.dataset == rhs.dataset &&
         (lhs.has_mixed_schema ? rhs.has_mixed_schema
                               : lhs.schema == rhs.schema) &&
         lhs.start == rhs.start && lhs.end == rhs.end &&
         lhs.limit == rhs.limit &&
         (lhs.has_mixed_stype_in ? rhs.has_mixed_stype_in
                                 : lhs.stype_in == rhs.stype_in) &&
         lhs.stype_out == rhs.stype_out && lhs.ts_out == rhs.ts_out &&
         lhs.symbol_cstr_len == rhs.symbol_cstr_len &&
         lhs.symbols == rhs.symbols && lhs.partial == rhs.partial &&
         lhs.not_found == rhs.not_found && lhs.mappings == rhs.mappings;
}
inline bool operator!=(const Metadata& lhs, const Metadata& rhs) {
  return !(lhs == rhs);
}

std::string ToString(const Metadata& metadata);
std::ostream& operator<<(std::ostream& stream, const Metadata& metadata);
std::string ToString(const SymbolMapping& mapping);
std::ostream& operator<<(std::ostream& stream, const SymbolMapping& mapping);
std::string ToString(const MappingInterval& interval);
std::ostream& operator<<(std::ostream& stream, const MappingInterval& interval);
}  // namespace databento
