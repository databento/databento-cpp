#pragma once

#include <cstdint>
#include <ostream>
#include <string>
#include <vector>

#include "databento/datetime.hpp"  // UnixNanos
#include "databento/enums.hpp"

namespace databento {
struct MappingInterval {
  // YYYYMMDD e.g. 2022-10-08 is represented as 20221008
  std::uint32_t start_date;
  // YYYYMMDD e.g. 2022-10-08 is represented as 20221008
  std::uint32_t end_date;
  std::string symbol;
};

struct SymbolMapping {
  // The native symbol.
  std::string native;
  // The mappings of `native` for different date ranges.
  std::vector<MappingInterval> intervals;
};

// Information about a DBN file or response.
struct Metadata {
  // The DBN schema version number.
  std::uint8_t version;
  // The dataset code.
  std::string dataset;
  // The data record schema. Specifies
  Schema schema;
  // The UNIX timestamp of the query start, or the first record if the file was
  // split.
  UnixNanos start;
  // The UNIX timestamp of the query end, or the last record if the file was
  // split.
  UnixNanos end;
  // The maximum number of records for the query.
  std::uint64_t limit;
  // The total number of records.
  std::uint64_t record_count;
  // The data compression format (if any).
  Compression compression;
  // The input symbology type.
  SType stype_in;
  // The output symbology type.
  SType stype_out;
  // The original query input symbols from the request.
  std::vector<std::string> symbols;
  // Symbols that did not resolve for _at least one day_ in the query time
  // range.
  std::vector<std::string> partial;
  // Symbols that did not resolve for _any_ day in the query time range.
  std::vector<std::string> not_found;
  // Symbol mappings containing a native symbol and its mapping intervals.
  std::vector<SymbolMapping> mappings;
};

inline bool operator==(const MappingInterval& lhs, const MappingInterval& rhs) {
  return lhs.start_date == rhs.start_date && lhs.end_date == rhs.end_date &&
         lhs.symbol == rhs.symbol;
}

inline bool operator==(const SymbolMapping& lhs, const SymbolMapping& rhs) {
  return lhs.native == rhs.native && lhs.intervals == rhs.intervals;
}

inline bool operator==(const Metadata& lhs, const Metadata& rhs) {
  return lhs.version == rhs.version && lhs.dataset == rhs.dataset &&
         lhs.schema == rhs.schema && lhs.start == rhs.start &&
         lhs.end == rhs.end && lhs.limit == rhs.limit &&
         lhs.record_count == rhs.record_count &&
         lhs.compression == rhs.compression && lhs.stype_in == rhs.stype_in &&
         lhs.stype_out == rhs.stype_out && lhs.symbols == rhs.symbols &&
         lhs.partial == rhs.partial && lhs.not_found == rhs.not_found &&
         lhs.mappings == rhs.mappings;
}

std::string ToString(const Metadata& metadata);
std::ostream& operator<<(std::ostream& stream, const Metadata& metadata);
std::string ToString(const SymbolMapping& mapping);
std::ostream& operator<<(std::ostream& stream, const SymbolMapping& mapping);
std::string ToString(const MappingInterval& interval);
std::ostream& operator<<(std::ostream& stream, const MappingInterval& interval);
}  // namespace databento
