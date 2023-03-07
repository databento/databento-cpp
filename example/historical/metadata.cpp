#include <cstddef>
#include <iostream>

#include "databento/constants.hpp"
#include "databento/enums.hpp"
#include "databento/historical.hpp"

int main() {
  using databento::dataset::kGlbxMdp3;

  auto client = databento::HistoricalBuilder{}.SetKeyFromEnv().Build();

  const auto publishers = client.MetadataListPublishers();
  std::cout << "Publishers:\n";
  for (const auto& publisher : publishers) {
    std::cout << "- " << publisher.first << ": " << publisher.second << '\n';
  }
  std::cout << '\n';

  const auto datasets = client.MetadataListDatasets();
  std::cout << "Datasets:\n";
  for (const auto& dataset : datasets) {
    std::cout << "- " << dataset << '\n';
  }
  std::cout << '\n';

  const auto schemas = client.MetadataListSchemas(kGlbxMdp3);
  std::cout << "Schemas(GLBX):\n";
  for (const auto& schema : schemas) {
    std::cout << "- " << schema << '\n';
  }
  std::cout << '\n';

  const auto fields = client.MetadataListFields(
      kGlbxMdp3, databento::Encoding::Dbn, databento::Schema::Trades);
  std::cout << "Fields:\n";
  const auto& dbn_trades_fields = fields.at("GLBX.MDP3")
                                      .at(databento::Encoding::Dbn)
                                      .at(databento::Schema::Trades);
  for (const auto& field_and_type : dbn_trades_fields) {
    std::cout << "- " << field_and_type.first << ": " << field_and_type.second
              << '\n';
  }
  std::cout << '\n';

  const auto encodings = client.MetadataListEncodings();
  std::cout << "Encodings:\n";
  for (const auto encoding : encodings) {
    std::cout << "- " << encoding << '\n';
  }
  std::cout << '\n';

  const auto dataset_conditions = client.MetadataListDatasetConditions(
      "GLBX.MDP3", "2019-06-01", "2019-08-01");
  std::cout << "Conditions:\n";
  for (const auto& dataset_condition : dataset_conditions) {
    std::cout << "- " << dataset_condition << "\n";
  }
  std::cout << '\n';

  const auto compressions = client.MetadataListCompressions();
  std::cout << "Compressions:\n";
  for (const auto compression : compressions) {
    std::cout << "- " << compression << '\n';
  }
  std::cout << '\n';

  const auto all_unit_prices = client.MetadataListUnitPrices(kGlbxMdp3);
  std::cout << "Unit prices:\n";
  for (const auto& mode_and_prices : all_unit_prices) {
    const auto* mode_str = ToString(mode_and_prices.first);
    for (const auto& schema_and_price : mode_and_prices.second) {
      std::cout << "- (" << mode_str << ", " << schema_and_price.first
                << "): " << schema_and_price.second << '\n';
    }
  }
  std::cout << '\n';

  const auto record_count = client.MetadataGetRecordCount(
      kGlbxMdp3, "2020-12-28", "2020-12-29", {"ESH1"}, databento::Schema::Mbo);
  std::cout << "Record count: " << record_count << "\n\n";

  const auto live_unit_prices =
      client.MetadataListUnitPrices(kGlbxMdp3, databento::FeedMode::Live);
  std::cout << "Unit prices (live):\n";
  for (const auto& schema_and_price : live_unit_prices) {
    std::cout << "- (" << schema_and_price.first
              << "): " << schema_and_price.second << '\n';
  }
  std::cout << '\n';

  const auto trades_unit_prices =
      client.MetadataListUnitPrices(kGlbxMdp3, databento::Schema::Trades);
  std::cout << "Unit prices (trades):\n";
  for (const auto& mode_and_price : trades_unit_prices) {
    std::cout << "- (" << mode_and_price.first << "): " << mode_and_price.second
              << '\n';
  }
  std::cout << '\n';

  const auto unit_price = client.MetadataListUnitPrices(
      kGlbxMdp3, databento::FeedMode::Historical, databento::Schema::Trades);
  std::cout << "Unit price (GLBX.MDP3, Historical, Trades): " << unit_price
            << "\n\n";

  const std::size_t billable_size = client.MetadataGetBillableSize(
      kGlbxMdp3, "2020-12-28", "2020-12-29", {"ESH1"}, databento::Schema::Mbo,
      databento::SType::Native, {});
  std::cout << "Billable size (uncompressed binary bytes): " << billable_size
            << "\n\n";

  const auto cost = client.MetadataGetCost(
      kGlbxMdp3, "2020-12-28", "2020-12-29", {"ESH1"}, databento::Schema::Mbo);
  std::cout << "Cost (in cents): " << cost << '\n';

  return 0;
}
