// Duplicate of the example usage code from the README.md to ensure
// it compiles and to be able to clang-format it.
// NOLINTBEGIN(google-build-using-namespace)
#include <databento/constants.hpp>
#include <databento/historical.hpp>
#include <iostream>

using namespace databento;

int main() {
  auto client = HistoricalBuilder{}.SetKey("$YOUR_API_KEY").Build();
  auto print_trades = [](const Record& record) {
    const auto& trade_msg = record.Get<TradeMsg>();
    std::cout << trade_msg << '\n';
    return KeepGoing::Continue;
  };
  client.TimeseriesGetRange("GLBX.MDP3",
                            {"2022-06-10T14:30", "2022-06-10T14:40"},
                            kAllSymbols, Schema::Trades, SType::RawSymbol,
                            SType::InstrumentId, {}, {}, print_trades);
}
// NOLINTEND(google-build-using-namespace)
