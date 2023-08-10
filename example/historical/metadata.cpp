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
    std::cout << "- " << publisher << '\n';
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

  const auto fields = client.MetadataListFields(databento::Encoding::Dbn,
                                                databento::Schema::Trades);
  std::cout << "Fields:\n";
  for (const auto& field_detail : fields) {
    std::cout << "- " << field_detail << '\n';
  }
  std::cout << '\n';

  const auto dataset_conditions = client.MetadataGetDatasetCondition(
      "GLBX.MDP3", {"2019-06-01", "2019-08-01"});
  std::cout << "Conditions:\n";
  for (const auto& dataset_condition : dataset_conditions) {
    std::cout << "- " << dataset_condition << "\n";
  }
  std::cout << '\n';

  const auto all_unit_prices = client.MetadataListUnitPrices(kGlbxMdp3);
  std::cout << "Unit prices:\n";
  for (const auto& mode_and_prices : all_unit_prices) {
    const auto* mode_str = ToString(mode_and_prices.mode);
    for (const auto& schema_and_price : mode_and_prices.unit_prices) {
      std::cout << "- (" << mode_str << ", " << schema_and_price.first
                << "): " << schema_and_price.second << '\n';
    }
  }
  std::cout << '\n';

  const auto record_count =
      client.MetadataGetRecordCount(kGlbxMdp3, {"2020-12-28", "2020-12-29"},
                                    {"ESH1"}, databento::Schema::Mbo);
  std::cout << "Record count: " << record_count << "\n\n";

  const std::size_t billable_size = client.MetadataGetBillableSize(
      kGlbxMdp3, {"2020-12-28", "2020-12-29"}, {"ESH1"}, databento::Schema::Mbo,
      databento::SType::RawSymbol, {});
  std::cout << "Billable size (uncompressed binary bytes): " << billable_size
            << "\n\n";

  const auto cost =
      client.MetadataGetCost(kGlbxMdp3, {"2020-12-28", "2020-12-29"}, {"ESH1"},
                             databento::Schema::Mbo);
  std::cout << "Cost (in cents): " << cost << '\n';

  return 0;
}
