#include <gtest/gtest.h>

#include "databento/enums.hpp"
#include "databento/metadata.hpp"

namespace databento::tests {
TEST(MetadataTests, TestDatasetConditionDetailToString) {
  const DatasetConditionDetail target{"2022-11-10", DatasetCondition::Available,
                                      "2023-03-01"};
  ASSERT_EQ(
      ToString(target),
      R"(DatasetConditionDetail { date = "2022-11-10", condition = available, last_modified_date = "2023-03-01" })");
}

TEST(MetadataTests, TestDatasetRangeToString) {
  const DatasetRange target{"2022-05-17T00:00:00.000000000Z",
                            "2023-01-07T00:00:00.000000000Z"};
  ASSERT_EQ(
      ToString(target),
      R"(DatasetRange { start = "2022-05-17T00:00:00.000000000Z", end = "2023-01-07T00:00:00.000000000Z" })");
}
}  // namespace databento::tests
