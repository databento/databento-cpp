#include <gtest/gtest.h>

#include <chrono>

#include "databento/datetime.hpp"
#include "scoped_period_separator_locale.hpp"

namespace databento::tests {
TEST(DateFromIso8601IntTests, TestValid) {
  ASSERT_EQ(databento::DateFromIso8601Int(20201110), "2020-11-10");
}

TEST(DateFromIso8601IntTests, TestPadding) {
  ASSERT_EQ(databento::DateFromIso8601Int(20190801), "2019-08-01");
}

TEST(DateFromIso8601IntTests, TestPeriodSeparatorLocale) {
  const ScopedPeriodSeparatorLocale locale;
  ASSERT_EQ(databento::DateFromIso8601Int(20201110), "2020-11-10");
}

TEST(ToIso8601Tests, TestPeriodSeparatorLocale) {
  const ScopedPeriodSeparatorLocale locale;
  ASSERT_EQ(ToIso8601(UnixNanos{std::chrono::nanoseconds{1'700'000'000'123'456'789}}),
            "2023-11-14T22:13:20.123456789Z");
}
}  // namespace databento::tests
