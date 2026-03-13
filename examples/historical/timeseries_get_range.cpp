#include <iostream>

#include "databento/constants.hpp"
#include "databento/enums.hpp"
#include "databento/historical.hpp"

namespace db = databento;

int main() {
  auto client = db::Historical::Builder().SetKeyFromEnv().Build();
  const auto limit = 1000;
  auto store = client.TimeseriesGetRange(
      db::dataset::kGlbxMdp3, db::DateTimeRange<std::string>{"2022-10-03"}, {"ESZ2"},
      db::Schema::Trades, db::SType::RawSymbol, db::SType::InstrumentId, limit);
  std::cout << store.GetMetadata() << '\n';
  while (const auto* record = store.NextRecord()) {
    const auto& trade_msg = record->Get<db::TradeMsg>();
    std::cout << trade_msg << '\n';
  }

  return 0;
}
