#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>  // setw

#include "databento/constants.hpp"
#include "databento/datetime.hpp"
#include "databento/dbn_file_store.hpp"
#include "databento/enums.hpp"
#include "databento/historical.hpp"
#include "databento/record.hpp"

int main() {
  auto client = databento::HistoricalBuilder{}.SetKeyFromEnv().Build();
  const auto limit = 1000;
  databento::DbnFileStore dbn_file_store = client.TimeseriesGetRangeToFile(
      databento::dataset::kGlbxMdp3, "2022-10-03T00:00", "2022-10-04T00:00",
      {"ESZ2"}, databento::Schema::Ohlcv1M, databento::SType::Native,
      databento::SType::ProductId, limit,
      "ESZ2-ohlcv1m-20201003-20201004.dbn.zst");
  dbn_file_store.Replay([](const databento::Record record) {
    const auto& ohlcv_bar = record.Get<databento::OhlcvMsg>();
    std::cout << ohlcv_bar << '\n';
    return databento::KeepGoing::Continue;
  });
  return 0;
}
