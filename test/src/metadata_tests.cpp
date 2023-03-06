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
      {{"2022-11-07", DatasetCondition::Available},
       {"2022-11-08", DatasetCondition::Bad},
       {"2022-11-09", DatasetCondition::Bad},
       {"2022-11-10", DatasetCondition::Available}}};
  const auto res = ToString(target);
  ASSERT_EQ(res, R"(DatasetConditionInfo {
    details = {
        DatasetConditionDetail { date = "2022-11-07", condition = available },
        DatasetConditionDetail { date = "2022-11-08", condition = bad },
        DatasetConditionDetail { date = "2022-11-09", condition = bad },
        DatasetConditionDetail { date = "2022-11-10", condition = available }
    }
})");
}
}  // namespace test
}  // namespace databento
