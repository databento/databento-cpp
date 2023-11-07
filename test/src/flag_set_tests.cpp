#include <gtest/gtest.h>

#include <sstream>

#include "databento/flag_set.hpp"

namespace databento {
namespace test {
TEST(FlagSetTests, TestBitwiseNot) {
  const FlagSet no_flags{};
  ASSERT_FALSE(no_flags.Any());
  const auto all_flags = ~no_flags;
  ASSERT_TRUE(all_flags.Any());
  ASSERT_TRUE((all_flags & FlagSet::kLast).Any());
  ASSERT_TRUE((all_flags & FlagSet::kMbp).Any());
  ASSERT_TRUE((all_flags & FlagSet::kBadTsRecv).Any());
}

TEST(FlagSetTests, TestBitwiseOr) {
  const FlagSet flag = FlagSet::kMbp;
  const FlagSet no_flags{};
  ASSERT_NE(flag, no_flags);
  ASSERT_EQ(flag, no_flags | FlagSet::kMbp);
}

TEST(FlagSetTests, TestBitwiseAnd) {
  const auto flag = static_cast<FlagSet>(0b10011000);
  ASSERT_TRUE(flag.Any());
  ASSERT_TRUE((flag & FlagSet::kLast).Any());
  ASSERT_TRUE((flag & FlagSet::kMbp).Any());
  ASSERT_TRUE((flag & FlagSet::kBadTsRecv).Any());
}

TEST(FlagSetTests, TestBitwiseAndAssignment) {
  FlagSet flag{};
  flag &= FlagSet::kLast;
  ASSERT_FALSE((flag & FlagSet::kLast).Any());
  flag = ~flag & FlagSet::kLast;
  ASSERT_TRUE((flag & FlagSet::kLast).Any());
}

TEST(FlagSetTests, TestBitwiseXor) {
  FlagSet flag = ~FlagSet{};
  flag ^= FlagSet::kLast;
  ASSERT_FALSE((flag & FlagSet::kLast).Any());
  ASSERT_TRUE((flag & FlagSet::kMbp).Any());
  ASSERT_TRUE((flag & FlagSet::kBadTsRecv).Any());
}

TEST(FlagSetTests, TestAny) {
  FlagSet flag{};
  ASSERT_FALSE(flag.Any());
  flag = FlagSet::kBadTsRecv;
  ASSERT_TRUE(flag.Any());
}

TEST(FlagSetTests, TestToString) {
  constexpr FlagSet kFlagSet = FlagSet::kMbp;
  std::ostringstream ss;
  ss << kFlagSet;
  ASSERT_EQ(ss.str(), "0b00010000");
}

TEST(FlagSetTests, TestConversionOperator) {
  constexpr FlagSet kFlagSet = FlagSet::kMbp | FlagSet::kTob;
  const auto raw = std::uint8_t{kFlagSet};
  ASSERT_EQ(raw, 0b01010000);
}
}  // namespace test
}  // namespace databento
