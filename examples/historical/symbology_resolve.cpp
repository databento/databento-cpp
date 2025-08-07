#include <iostream>
#include <vector>

#include "databento/datetime.hpp"
#include "databento/enums.hpp"
#include "databento/historical.hpp"
#include "databento/symbology.hpp"

namespace db = databento;

int main(int argc, char* argv[]) {
  if (argc < 6) {
    std::cerr << "USAGE: symbology-resolve <DATASET> <STYPE_IN> <STYPE_OUT> "
                 "<DATE> <SYMBOLS...>\n";
    return 1;
  }
  const auto stype_in = db::FromString<db::SType>(argv[2]);
  const auto stype_out = db::FromString<db::SType>(argv[3]);

  std::vector<std::string> symbols;
  for (int i = 5; i < argc; ++i) {
    symbols.emplace_back(argv[i]);
  }

  auto client = db::Historical::Builder().SetKeyFromEnv().Build();
  const db::SymbologyResolution resolution = client.SymbologyResolve(
      argv[1], symbols, stype_in, stype_out, db::DateTimeRange<std::string>{argv[4]});
  std::cout << resolution << '\n';

  return 0;
}
