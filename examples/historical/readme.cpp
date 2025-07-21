// Duplicate of the example usage code from the README.md to ensure
// it compiles and to be able to clang-format it.
// NOLINTBEGIN(google-build-using-namespace)
#include <databento/dbn.hpp>
#include <databento/historical.hpp>
#include <databento/symbol_map.hpp>
#include <iostream>

using namespace databento;

int main() {
  auto client = HistoricalBuilder{}.SetKey("$YOUR_API_KEY").Build();
  TsSymbolMap symbol_map;
  auto decode_symbols = [&symbol_map](const Metadata& metadata) {
    symbol_map = metadata.CreateSymbolMap();
  };
  auto print_trades = [&symbol_map](const Record& record) {
    const auto& trade_msg = record.Get<TradeMsg>();
    std::cout << "Received trade for " << symbol_map.At(trade_msg) << ": " << trade_msg
              << '\n';
    return KeepGoing::Continue;
  };
  client.TimeseriesGetRange("GLBX.MDP3", {"2022-06-10T14:30", "2022-06-10T14:40"},
                            {"ESM2", "NQZ2"}, Schema::Trades, SType::RawSymbol,
                            SType::InstrumentId, {}, decode_symbols, print_trades);
}
// NOLINTEND(google-build-using-namespace)
