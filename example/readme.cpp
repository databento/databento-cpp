// Duplicate of the example usage code from the README.md to ensure
// it compiles and to be able to clang-format it.
#include <chrono>
#include <ctime>
#include <databento/historical.hpp>
#include <iomanip>
#include <iostream>

using namespace databento;

static constexpr auto kApiKey = "YOUR_API_KEY";

int main() {
  auto client = HistoricalBuilder{}.SetKey(kApiKey).Build();
  client.TimeseriesStream(
      "GLBX.MDP3", "2022-06-10T14:30", "2022-05-10T14:40", {"ES"}, Schema::Mbo,
      SType::Smart, SType::ProductId, {}, {}, [](const Record& record) {
        const auto& trade_msg = record.get<TradeMsg>();
        std::cout << trade_msg.hd.product_id << ": " << trade_msg.size << " @ "
                  << trade_msg.price << std::endl;
        return KeepGoing::Continue;
      });
}
