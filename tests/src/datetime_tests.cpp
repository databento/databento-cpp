#include <gtest/gtest.h>

#include "databento/datetime.hpp"

namespace databento::tests {
TEST(DateFromIso8601IntTests, TestValid) {
  ASSERT_EQ(databento::DateFromIso8601Int(20201110), "2020-11-10");
}

TEST(DateFromIso8601IntTests, TestPadding) {
  ASSERT_EQ(databento::DateFromIso8601Int(20190801), "2019-08-01");
}
}  // namespace databento::tests
