// Duplicate of the example usage code from the README.md to ensure
// it compiles and to be able to clang-format it.
#include <databento/dbn.hpp>
#include <databento/historical.hpp>
#include <databento/symbol_map.hpp>
#include <iostream>

namespace db = databento;

int main() {
  auto client = db::Historical::Builder().SetKey("$YOUR_API_KEY").Build();
  db::TsSymbolMap symbol_map;
  auto decode_symbols = [&symbol_map](const db::Metadata& metadata) {
    symbol_map = metadata.CreateSymbolMap();
  };
  auto print_trades = [&symbol_map](const db::Record& record) {
    const auto& trade_msg = record.Get<db::TradeMsg>();
    std::cout << "Received trade for " << symbol_map.At(trade_msg) << ": " << trade_msg
              << '\n';
    return db::KeepGoing::Continue;
  };
  client.TimeseriesGetRange("GLBX.MDP3", {"2022-06-10T14:30", "2022-06-10T14:40"},
                            {"ESM2", "NQZ2"}, db::Schema::Trades, db::SType::RawSymbol,
                            db::SType::InstrumentId, {}, decode_symbols, print_trades);
}
