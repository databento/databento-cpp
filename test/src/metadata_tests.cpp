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

TEST(MetadataTests, TestDatasetConditionInfoToString) {
  const DatasetConditionInfo target{
      DatasetCondition::Bad,
      {{"2022-11-07", DatasetCondition::Available},
       {"2022-11-08", DatasetCondition::Bad},
       {"2022-11-09", DatasetCondition::Bad},
       {"2022-11-10", DatasetCondition::Available}},
      "2022-11-07",
      "2022-11-10",
      "2017-05-21",
      "2023-03-01"};
  const auto res = ToString(target);
  ASSERT_EQ(res, R"(DatasetConditionInfo {
    condition = bad,
    details = {
        DatasetConditionDetail { date = "2022-11-07", condition = available },
        DatasetConditionDetail { date = "2022-11-08", condition = bad },
        DatasetConditionDetail { date = "2022-11-09", condition = bad },
        DatasetConditionDetail { date = "2022-11-10", condition = available }
    },
    adjusted_start_date = "2022-11-07",
    adjusted_end_date = "2022-11-10",
    available_start_date = "2017-05-21",
    available_end_date = "2023-03-01"
})");
}
}  // namespace test
}  // namespace databento
