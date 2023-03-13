#include <gtest/gtest.h>

#include "databento/enums.hpp"
#include "databento/metadata.hpp"

namespace databento {
namespace test {
TEST(MetadataTests, TestDatasetConditionDetailToString) {
  const DatasetConditionDetail target{"2022-11-10",
                                      DatasetCondition::Available};
  ASSERT_EQ(
      ToString(target),
      R"(DatasetConditionDetail { date = "2022-11-10", condition = available })");
}

TEST(MetadataTests, TestDatasetRangeToString) {
  const DatasetRange target{"2022-05-17", "2023-01-07"};
  ASSERT_EQ(
      ToString(target),
      R"(DatasetRange { start_date = "2022-05-17", end_date = "2023-01-07" })");
}
}  // namespace test
}  // namespace databento
