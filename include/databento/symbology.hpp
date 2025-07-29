#pragma once

#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "databento/dbn.hpp"
#include "databento/enums.hpp"
#include "databento/symbol_map.hpp"

namespace databento {
// Sentinel value for requesting all symbols
static const std::vector<std::string> kAllSymbols{"ALL_SYMBOLS"};

struct SymbologyResolution {
  std::unordered_map<std::string, std::vector<MappingInterval>> mappings;
  std::vector<std::string> partial;
  std::vector<std::string> not_found;
  SType stype_in;
  SType stype_out;

  TsSymbolMap CreateSymbolMap() const;
};

// Converts a vector of symbols to a comma-delineated string for sending to
// Databento's historical and live APIs.
//
// Throws InvalidArgumentError if symbols is empty or the iterator range is
// empty.
std::string JoinSymbolStrings(const std::string& method_name,
                              std::vector<std::string>::const_iterator symbols_begin,
                              std::vector<std::string>::const_iterator symbols_end);
std::string JoinSymbolStrings(const std::string& method_name,
                              const std::vector<std::string>& symbols);
std::string ToString(const SymbologyResolution& sym_res);
std::ostream& operator<<(std::ostream& stream, const SymbologyResolution& sym_res);
}  // namespace databento
