// Duplicate of the example usage code from the README.md to ensure
// it compiles and to be able to clang-format it.
#include <chrono>
#include <databento/live.hpp>
#include <databento/symbol_map.hpp>
#include <iostream>
#include <thread>

namespace db = databento;

int main() {
  db::PitSymbolMap symbol_mappings;

  auto client = db::LiveThreaded::Builder()
                    .SetKeyFromEnv()
                    .SetDataset(db::Dataset::GlbxMdp3)
                    .BuildThreaded();

  auto handler = [&symbol_mappings](const db::Record& rec) {
    symbol_mappings.OnRecord(rec);
    if (const auto* trade = rec.GetIf<db::TradeMsg>()) {
      std::cout << "Received trade for " << symbol_mappings[trade->hd.instrument_id]
                << ':' << *trade << '\n';
    }
    return db::KeepGoing::Continue;
  };

  client.Subscribe({"ES.FUT"}, db::Schema::Trades, db::SType::Parent);
  client.Start(handler);
  std::this_thread::sleep_for(std::chrono::seconds{10});
  return 0;
}
