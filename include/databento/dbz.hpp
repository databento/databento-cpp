#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "databento/datetime.hpp"
#include "databento/enums.hpp"
#include "databento/symbology.hpp"

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

// Information about a DBZ file or response.
struct Metadata {
  // The DBZ schema version number.
  std::uint8_t version;
  // The dataset name.
  std::string dataset;
  // The data record schema. Specifies
  Schema schema;
  // The UNIX timestamp of the query start, or the first record if the file was
  // split.
  EpochNanos start;
  // The UNIX timestamp of the query end, or the last record if the file was
  // split.
  EpochNanos end;
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
}  // namespace databento
