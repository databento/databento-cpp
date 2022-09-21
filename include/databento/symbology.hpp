#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace databento {
struct MappingInterval {
  std::string start_date;
  std::string end_date;
  std::string symbol;
};

struct SymbologyResolution {
  std::unordered_map<std::string, std::vector<MappingInterval>> mappings;
  std::vector<std::string> partial;
  std::vector<std::string> not_found;
};
}  // namespace databento
