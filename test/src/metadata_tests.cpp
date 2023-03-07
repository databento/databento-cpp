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
}  // namespace test
}  // namespace databento
