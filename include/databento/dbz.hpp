#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "databento/datetime.hpp"
#include "databento/enums.hpp"
#include "databento/symbology.hpp"

namespace databento {
struct SymbolMapping {
  std::string native;
  std::unordered_map<std::string, std::vector<MappingInterval>> mappings;
};

struct Metadata {
  std::uint8_t version;
  std::string dataset;
  Schema schema;
  EpochNanos start;
  EpochNanos end;
  std::uint64_t limit;
  std::uint64_t record_count;
  Compression compression;
  SType stype_in;
  SType stype_out;
  std::vector<std::string> symbols;
  std::vector<std::string> partial;
  std::vector<std::string> not_found;
  // mappings
};
}  // namespace databento
