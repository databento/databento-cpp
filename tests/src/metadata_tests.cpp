#include <gtest/gtest.h>

#include "databento/enums.hpp"
#include "databento/metadata.hpp"

namespace databento::tests {
TEST(MetadataTests, TestDatasetConditionDetailToString) {
  const DatasetConditionDetail available{"2022-11-10", DatasetCondition::Available,
                                         "2023-03-01"};
  EXPECT_EQ(
      ToString(available),
      R"(DatasetConditionDetail { date = "2022-11-10", condition = available, last_modified_date = "2023-03-01" })");
  const DatasetConditionDetail missing{"2022-11-11", DatasetCondition::Missing, {}};
  EXPECT_EQ(
      ToString(missing),
      R"(DatasetConditionDetail { date = "2022-11-11", condition = missing, last_modified_date = nullopt })");
}

TEST(MetadataTests, TestDatasetRangeToString) {
  const DatasetRange target{
      "2022-05-17T00:00:00.000000000Z",
      "2023-01-07T00:00:00.000000000Z",
      {{Schema::Bbo1M,
        {"2020-08-02T00:00:00.000000000Z", "2023-03-23T00:00:00.000000000Z"}},
       {Schema::Bbo1S,
        {"2020-08-02T00:00:00.000000000Z", "2023-03-23T00:00:00.000000000Z"}}}};
  ASSERT_EQ(ToString(target),
            R"(DatasetRange {
    start = "2022-05-17T00:00:00.000000000Z",
    end = "2023-01-07T00:00:00.000000000Z",
    range_by_schema = {
        bbo-1s: DateTimeRange { start = "2020-08-02T00:00:00.000000000Z", end = "2023-03-23T00:00:00.000000000Z" },
        bbo-1m: DateTimeRange { start = "2020-08-02T00:00:00.000000000Z", end = "2023-03-23T00:00:00.000000000Z" }
    }
})");
}
}  // namespace databento::tests
