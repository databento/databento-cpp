#include <cstddef>
#include <iostream>

#include "databento/enums.hpp"
#include "databento/historical.hpp"

int main() {
  auto client = databento::HistoricalBuilder{}.keyFromEnv().Build();

  const auto publishers = client.MetadataListPublishers();
  std::cout << "Publishers:" << std::endl;
  for (const auto& publisher : publishers) {
    std::cout << "- " << publisher.first << ": " << publisher.second
              << std::endl;
  }
  std::cout << std::endl;

  const auto datasets = client.MetadataListDatasets();
  std::cout << "Datasets:" << std::endl;
  for (const auto& dataset : datasets) {
    std::cout << "- " << dataset << std::endl;
  }
  std::cout << std::endl;

  // const auto schemas = client.MetadataListSchemas("GLBX.MDP3", "2020-01-01",
  // "2022-01-01");
  const auto schemas = client.MetadataListSchemas("GLBX.MDP3");
  std::cout << "Schemas(GLBX): " << std::endl;
  for (const auto& schema : schemas) {
    std::cout << "- " << databento::ToString(schema) << std::endl;
  }
  std::cout << std::endl;

  const auto all_unit_prices = client.MetadataListUnitPrices("GLBX.MDP3");
  std::cout << "Unit prices: " << std::endl;
  for (const auto& mode_and_prices : all_unit_prices) {
    const auto* mode_str = ToString(mode_and_prices.first);
    for (const auto& schema_and_price : mode_and_prices.second) {
      std::cout << "- (" << mode_str << ", " << ToString(schema_and_price.first)
                << "): " << schema_and_price.second << std::endl;
    }
  }
  std::cout << std::endl;

  const auto live_unit_prices =
      client.MetadataListUnitPrices("GLBX.MDP3", databento::FeedMode::Live);
  std::cout << "Unit prices (live): " << std::endl;
  for (const auto& schema_and_price : live_unit_prices) {
    std::cout << "- (" << ToString(schema_and_price.first)
              << "): " << schema_and_price.second << std::endl;
  }
  std::cout << std::endl;

  const auto trades_unit_prices =
      client.MetadataListUnitPrices("GLBX.MDP3", databento::Schema::Trades);
  std::cout << "Unit prices (trades): " << std::endl;
  for (const auto& mode_and_price : trades_unit_prices) {
    std::cout << "- (" << ToString(mode_and_price.first)
              << "): " << mode_and_price.second << std::endl;
  }
  std::cout << std::endl;

  const auto unit_price = client.MetadataListUnitPrices(
      "GLBX.MDP3", databento::FeedMode::Historical, databento::Schema::Trades);
  std::cout << "Unit price (GLBX.MDP3, Historical, Trades): " << unit_price
            << std::endl
            << std::endl;

  const std::size_t billable_size = client.MetadataGetBillableSize(
      "GLBX.MDP3", "2020-12-28", "2020-12-29", {"ESH0"}, databento::Schema::Mbo,
      databento::SType::Native, {});
  std::cout << "Billable size: " << billable_size << std::endl << std::endl;

  return 0;
}
