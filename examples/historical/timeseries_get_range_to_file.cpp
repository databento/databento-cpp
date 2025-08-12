#include "databento/constants.hpp"
#include "databento/datetime.hpp"
#include "databento/dbn_file_store.hpp"
#include "databento/enums.hpp"
#include "databento/historical.hpp"
#include "databento/record.hpp"

namespace db = databento;

int main() {
  auto client = db::Historical::Builder().SetKeyFromEnv().Build();
  const auto limit = 1000;
  db::DbnFileStore dbn_file_store = client.TimeseriesGetRangeToFile(
      db::dataset::kGlbxMdp3, {"2022-10-03T00:00", "2022-10-04T00:00"}, {"ESZ2"},
      db::Schema::Ohlcv1M, db::SType::RawSymbol, db::SType::InstrumentId, limit,
      "ESZ2-ohlcv1m-20201003-20201004.dbn.zst");
  dbn_file_store.Replay([](const db::Record record) {
    const auto& ohlcv_bar = record.Get<db::OhlcvMsg>();
    std::cout << ohlcv_bar << '\n';
    return db::KeepGoing::Continue;
  });
  return 0;
}
