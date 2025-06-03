// Duplicate of the example usage code from the README.md to ensure
// it compiles and to be able to clang-format it.
// NOLINTBEGIN(google-build-using-namespace)
#include <chrono>
#include <databento/live.hpp>
#include <databento/symbol_map.hpp>
#include <iostream>
#include <thread>

using namespace databento;

int main() {
  PitSymbolMap symbol_mappings;

  auto client = LiveBuilder{}
                    .SetKeyFromEnv()
                    .SetDataset(Dataset::GlbxMdp3)
                    .BuildThreaded();

  auto handler = [&symbol_mappings](const Record& rec) {
    symbol_mappings.OnRecord(rec);
    if (const auto* trade = rec.GetIf<TradeMsg>()) {
      std::cout << "Received trade for "
                << symbol_mappings[trade->hd.instrument_id] << ':' << *trade
                << '\n';
    }
    return KeepGoing::Continue;
  };

  client.Subscribe({"ES.FUT"}, Schema::Trades, SType::Parent);
  client.Start(handler);
  std::this_thread::sleep_for(std::chrono::seconds{10});
  return 0;
}
// NOLINTEND(google-build-using-namespace)
