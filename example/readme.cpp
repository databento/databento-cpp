// Duplicate of the example usage code from the README.md to ensure
// it compiles and to be able to clang-format it.
// NOLINTBEGIN(google-build-using-namespace)
#include <databento/historical.hpp>
#include <iostream>

using namespace databento;

static constexpr auto kApiKey = "YOUR_API_KEY";

int main() {
  auto client = HistoricalBuilder{}.SetKey(kApiKey).Build();
  client.TimeseriesStream("GLBX.MDP3", "2022-06-10", "2022-06-11", {"ES"},
                          Schema::Trades, SType::Smart, SType::ProductId, {},
                          {}, [](const Record& record) {
                            const auto& trade_msg = record.get<TradeMsg>();
                            std::cout << trade_msg << '\n';
                            return KeepGoing::Continue;
                          });
}
// NOLINTEND(google-build-using-namespace)
