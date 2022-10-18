#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace databento {
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
