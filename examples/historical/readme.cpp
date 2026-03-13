// Duplicate of the example usage code from the README.md to ensure
// it compiles and to be able to clang-format it.
#include <databento/dbn.hpp>
#include <databento/historical.hpp>
#include <databento/symbol_map.hpp>
#include <iostream>

namespace db = databento;

int main() {
  auto client = db::Historical::Builder().SetKey("$YOUR_API_KEY").Build();
  auto store =
      client.TimeseriesGetRange("GLBX.MDP3", {"2022-06-10T14:30", "2022-06-10T14:40"},
                                {"ESM2", "NQZ2"}, db::Schema::Trades);
  auto symbol_map = store.GetMetadata().CreateSymbolMap();
  while (const auto* record = store.NextRecord()) {
    const auto& trade_msg = record->Get<db::TradeMsg>();
    std::cout << "Received trade for " << symbol_map.At(trade_msg) << ": " << trade_msg
              << '\n';
  }
}
