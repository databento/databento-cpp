#include <iostream>  // setw

#include "databento/constants.hpp"
#include "databento/enums.hpp"
#include "databento/historical.hpp"

int main() {
  auto client = databento::HistoricalBuilder{}.SetKeyFromEnv().Build();
  const auto limit = 1000;
  client.TimeseriesGetRange(
      databento::dataset::kGlbxMdp3,
      databento::DateTimeRange<std::string>{"2022-10-03"}, {"ESZ2"},
      databento::Schema::Trades, databento::SType::RawSymbol,
      databento::SType::InstrumentId, limit,
      [](databento::Metadata&& metadata) { std::cout << metadata << '\n'; },
      [](const databento::Record& record) {
        const auto& trade_msg = record.Get<databento::TradeMsg>();
        std::cout << trade_msg << '\n';
        return databento::KeepGoing::Continue;
      });

  return 0;
}
