#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace databento {
// Sentinel value for requesting all symbols
static const std::vector<std::string> kAllSymbols{};

struct StrMappingInterval {
  // YYYY-MM-DD
  std::string start_date;
  // YYYY-MM-DD
  std::string end_date;
  std::string symbol;
};

struct SymbologyResolution {
  std::unordered_map<std::string, std::vector<StrMappingInterval>> mappings;
  std::vector<std::string> partial;
  std::vector<std::string> not_found;
};
}  // namespace databento
