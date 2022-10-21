#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>  // setw

#include "databento/constants.hpp"
#include "databento/datetime.hpp"
#include "databento/enums.hpp"
#include "databento/file_bento.hpp"
#include "databento/historical.hpp"
#include "databento/record.hpp"

int main() {
  auto client = databento::HistoricalBuilder{}.SetKeyFromEnv().Build();
  const auto limit = 1000;
  databento::FileBento file_bento = client.TimeseriesStreamToFile(
      databento::dataset::kGlbxMdp3, "2022-10-03T00:00", "2022-10-04T00:00",
      {"ESZ2"}, databento::Schema::Ohlcv1M, databento::SType::Native,
      databento::SType::ProductId, limit, "ESZ2-ohlcv1m-20201003-20201004.dbz");
  file_bento.Replay([](const databento::Record record) {
    const auto& ohlcv_bar = record.get<databento::OhlcvMsg>();
    std::cout << "open: " << ohlcv_bar.high << " close: " << ohlcv_bar.close
              << std::endl;
    return databento::KeepGoing::Continue;
  });
  return 0;
}
