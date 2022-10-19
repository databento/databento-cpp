// Duplicate of the example usage code from the README.md to ensure
// it compiles and to be able to clang-format it.
#include <chrono>
#include <ctime>
#include <databento/historical.hpp>
#include <iomanip>
#include <iostream>

using namespace databento;

static constexpr auto kApiKey = "YOUR_API_KEY";

EpochNanos ToEpochNanos(int year, int month, int day, int hour, int min) {
  std::tm time{};
  time.tm_year = year - 1900;
  time.tm_mon = month - 1;
  time.tm_mday = day;
  time.tm_hour = hour;
  time.tm_min = min;
  return EpochNanos{std::chrono::seconds{timegm(&time)}};
}

int main() {
  auto start = ToEpochNanos(2022, 6, 10, 14, 30);
  auto end = ToEpochNanos(2022, 6, 10, 14, 40);
  auto client = HistoricalBuilder{}.key(kApiKey).Build();
  client.TimeseriesStream(
      "GLBX.MDP3", {"ES"}, Schema::Mbo, start, end, SType::Smart,
      SType::ProductId, {}, [](Metadata&&) {},
      [](const Record& record) {
        const auto& trade_msg = record.get<TradeMsg>();
        std::cout << trade_msg.hd.product_id << ": " << trade_msg.size << " @ "
                  << trade_msg.price << std::endl;
        return KeepGoing::Continue;
      });
}
