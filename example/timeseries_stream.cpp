#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>  // setw

#include "databento/datetime.hpp"
#include "databento/enums.hpp"
#include "databento/historical.hpp"

// Converts a date to Unix Epoch nanoseconds
databento::UnixNanos DateToUnixNanos(int year, int month, int day) {
  std::tm time{};
  time.tm_year = year - 1900;
  // January is 0
  time.tm_mon = month - 1;
  time.tm_mday = day;
  return databento::UnixNanos{std::chrono::seconds{::timegm(&time)}};
}

int main() {
  auto client = databento::HistoricalBuilder{}.SetKeyFromEnv().Build();
  const databento::UnixNanos start = DateToUnixNanos(2022, 10, 3);
  const databento::UnixNanos end = DateToUnixNanos(2022, 10, 4);
  const auto limit = 1000;
  client.TimeseriesStream(
      "GLBX.MDP3", start, end, {"ESZ2"}, databento::Schema::Trades,
      databento::SType::Native, databento::SType::ProductId, limit,
      [](databento::Metadata&& metadata) {
        std::cout << "Metadata (" << metadata.dataset << ", " << metadata.schema
                  << "): " << metadata.record_count << " records" << std::endl;
      },
      [](const databento::Record& record) {
        const auto& trade_msg = record.get<databento::TradeMsg>();
        std::cout << "Received trade " << std::setw(3) << trade_msg.size
                  << " @ " << trade_msg.price << std::endl;
        return databento::KeepGoing::Continue;
      });

  return 0;
}
