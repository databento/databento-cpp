#include <sysexits.h>

#include <iostream>
#include <vector>

#include "databento/enums.hpp"
#include "databento/historical.hpp"
#include "databento/symbology.hpp"

int main(int argc, char* argv[]) {
  if (argc < 6) {
    std::cerr << "USAGE: symbology-resolve <DATASET> <STYPE_IN> <STYPE_OUT> "
                 "<DATE> <SYMBOLS...>"
              << std::endl;
    return EX_USAGE;
  }
  const auto stype_in = databento::FromString<databento::SType>(argv[2]);
  const auto stype_out = databento::FromString<databento::SType>(argv[3]);

  std::vector<std::string> symbols;
  for (int i = 5; i < argc; ++i) {
    symbols.emplace_back(argv[i]);
  }

  auto client = databento::HistoricalBuilder{}.keyFromEnv().Build();
  const databento::SymbologyResolution resolution = client.SymbologyResolve(
      argv[1], symbols, stype_in, stype_out, argv[4], argv[4]);
  if (!resolution.not_found.empty()) {
    std::cout << "Not found:" << std::endl;
    for (const auto& symbol : resolution.not_found) {
      std::cout << "- " << symbol << std::endl;
    }
    std::cout << std::endl;
  }
  if (!resolution.partial.empty()) {
    std::cout << "Partial:" << std::endl;
    for (const auto& symbol : resolution.partial) {
      std::cout << "- " << symbol << std::endl;
    }
    std::cout << std::endl;
  }
  if (!resolution.mappings.empty()) {
    std::cout << "Resolved:" << std::endl;
    for (const auto& mapping : resolution.mappings) {
      std::cout << "- " << mapping.first << " -> "
                << mapping.second.front().symbol << std::endl;
    }
  }

  return 0;
}