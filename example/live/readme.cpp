// Duplicate of the example usage code from the README.md to ensure
// it compiles and to be able to clang-format it.
// NOLINTBEGIN(google-build-using-namespace)
#include <chrono>
#include <databento/live.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>

using namespace databento;

int main() {
  std::unordered_map<std::uint32_t, std::string> symbol_mappings;

  auto client = LiveBuilder{}
                    .SetKey("$YOUR_API_KEY")
                    .SetDataset("GLBX.MDP3")
                    .BuildThreaded();

  auto handler = [&symbol_mappings](const Record& rec) {
    if (rec.Holds<TradeMsg>()) {
      auto trade = rec.Get<TradeMsg>();
      std::cout << "Received trade for "
                << symbol_mappings[trade.hd.instrument_id] << ':' << trade
                << '\n';
    } else if (rec.Holds<SymbolMappingMsg>()) {
      auto mapping = rec.Get<SymbolMappingMsg>();
      symbol_mappings[mapping.hd.instrument_id] =
          mapping.stype_out_symbol.data();
    }
    return KeepGoing::Continue;
  };

  client.Subscribe({"ES.FUT"}, Schema::Trades, SType::Parent);
  client.Start(handler);
  std::this_thread::sleep_for(std::chrono::seconds{10});
  return 0;
}
// NOLINTEND(google-build-using-namespace)
