#include <sysexits.h>

#include <iostream>
#include <vector>

#include "databento/enums.hpp"
#include "databento/historical.hpp"
#include "databento/symbology.hpp"

int main(int argc, char* argv[]) {
  if (argc < 6) {
    std::cerr << "USAGE: symbology-resolve <DATASET> <STYPE_IN> <STYPE_OUT> "
                 "<START_DATE> <END_DATE> <SYMBOLS...>\n";
    return EX_USAGE;
  }
  const auto stype_in = databento::FromString<databento::SType>(argv[2]);
  const auto stype_out = databento::FromString<databento::SType>(argv[3]);

  std::vector<std::string> symbols;
  for (int i = 6; i < argc; ++i) {
    symbols.emplace_back(argv[i]);
  }

  auto client = databento::HistoricalBuilder{}.SetKeyFromEnv().Build();
  const databento::SymbologyResolution resolution = client.SymbologyResolve(
      argv[1], argv[4], argv[5], symbols, stype_in, stype_out);
  std::cout << resolution << '\n';

  return 0;
}
