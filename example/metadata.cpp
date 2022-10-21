#include <cstddef>
#include <iostream>

#include "databento/constants.hpp"
#include "databento/enums.hpp"
#include "databento/historical.hpp"

int main() {
  using databento::dataset::kGlbxMdp3;

  auto client = databento::HistoricalBuilder{}.SetKeyFromEnv().Build();

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

  const auto schemas = client.MetadataListSchemas(kGlbxMdp3);
  std::cout << "Schemas(GLBX):" << std::endl;
  for (const auto& schema : schemas) {
    std::cout << "- " << schema << std::endl;
  }
  std::cout << std::endl;

  const auto fields = client.MetadataListFields(
      kGlbxMdp3, databento::Encoding::Dbz, databento::Schema::Trades);
  std::cout << "Fields:" << std::endl;
  const auto& dbz_trades_fields = fields.at("GLBX.MDP3")
                                      .at(databento::Encoding::Dbz)
                                      .at(databento::Schema::Trades);
  for (const auto& field_and_type : dbz_trades_fields) {
    std::cout << "- " << field_and_type.first << ": " << field_and_type.second
              << std::endl;
  }
  std::cout << std::endl;

  const auto encodings = client.MetadataListEncodings();
  std::cout << "Encodings:" << std::endl;
  for (const auto encoding : encodings) {
    std::cout << "- " << encoding << std::endl;
  }
  std::cout << std::endl;

  const auto compressions = client.MetadataListCompressions();
  std::cout << "Compressions:" << std::endl;
  for (const auto compression : compressions) {
    std::cout << "- " << compression << std::endl;
  }
  std::cout << std::endl;

  const auto all_unit_prices = client.MetadataListUnitPrices(kGlbxMdp3);
  std::cout << "Unit prices:" << std::endl;
  for (const auto& mode_and_prices : all_unit_prices) {
    const auto* mode_str = ToString(mode_and_prices.first);
    for (const auto& schema_and_price : mode_and_prices.second) {
      std::cout << "- (" << mode_str << ", " << schema_and_price.first
                << "): " << schema_and_price.second << std::endl;
    }
  }
  std::cout << std::endl;

  const auto live_unit_prices =
      client.MetadataListUnitPrices(kGlbxMdp3, databento::FeedMode::Live);
  std::cout << "Unit prices (live):" << std::endl;
  for (const auto& schema_and_price : live_unit_prices) {
    std::cout << "- (" << schema_and_price.first
              << "): " << schema_and_price.second << std::endl;
  }
  std::cout << std::endl;

  const auto trades_unit_prices =
      client.MetadataListUnitPrices(kGlbxMdp3, databento::Schema::Trades);
  std::cout << "Unit prices (trades):" << std::endl;
  for (const auto& mode_and_price : trades_unit_prices) {
    std::cout << "- (" << mode_and_price.first << "): " << mode_and_price.second
              << std::endl;
  }
  std::cout << std::endl;

  const auto unit_price = client.MetadataListUnitPrices(
      kGlbxMdp3, databento::FeedMode::Historical, databento::Schema::Trades);
  std::cout << "Unit price (GLBX.MDP3, Historical, Trades): " << unit_price
            << std::endl
            << std::endl;

  const std::size_t billable_size = client.MetadataGetBillableSize(
      kGlbxMdp3, "2020-12-28", "2020-12-29", {"ESH0"}, databento::Schema::Mbo,
      databento::SType::Native, {});
  std::cout << "Billable size: " << billable_size << std::endl << std::endl;

  return 0;
}
