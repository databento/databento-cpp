#include <iostream>

#include "databento/constants.hpp"
#include "databento/enums.hpp"
#include "databento/historical.hpp"

namespace db = databento;

int main() {
  auto client = db::Historical::Builder().SetKeyFromEnv().Build();
  const auto limit = 1000;
  client.TimeseriesGetRange(
      db::dataset::kGlbxMdp3, db::DateTimeRange<std::string>{"2022-10-03"}, {"ESZ2"},
      db::Schema::Trades, db::SType::RawSymbol, db::SType::InstrumentId, limit,
      [](db::Metadata&& metadata) { std::cout << metadata << '\n'; },
      [](const db::Record& record) {
        const auto& trade_msg = record.Get<db::TradeMsg>();
        std::cout << trade_msg << '\n';
        return db::KeepGoing::Continue;
      });

  return 0;
}
