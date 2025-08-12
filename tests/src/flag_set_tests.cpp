#include <gtest/gtest.h>

#include "databento/flag_set.hpp"

namespace databento::tests {
TEST(FlagSetTests, TestBasic) {
  const FlagSet no_flags{};
  EXPECT_FALSE(no_flags.Any());
  EXPECT_TRUE(no_flags.IsEmpty());
  const FlagSet all_flags{255};
  EXPECT_TRUE(all_flags.Any());
  EXPECT_FALSE(all_flags.IsEmpty());
  EXPECT_TRUE((all_flags.IsLast()));
  EXPECT_TRUE((all_flags.IsMbp()));
  EXPECT_TRUE((all_flags.IsBadTsRecv()));
}

TEST(FlagSetTests, TestAny) {
  FlagSet flag{};
  ASSERT_FALSE(flag.Any());
  flag.SetBadTsRecv();
  ASSERT_TRUE(flag.Any());
}

TEST(FlagSetTests, TestConversionOperator) {
  constexpr FlagSet kFlagSet{FlagSet::kMbp | FlagSet::kTob};
  const auto raw = std::uint8_t{kFlagSet};
  ASSERT_EQ(raw, 0b01010000);
}

TEST(FlagSetTests, ToStringEmpty) {
  constexpr FlagSet kTarget{};
  ASSERT_EQ(ToString(kTarget), "0");
}

TEST(FlagSetTests, ToStringOneSet) {
  const auto target = FlagSet{}.SetMbp();
  ASSERT_EQ(ToString(target), "MBP (16)");
}

TEST(FlagSetTests, ToStringThreeSet) {
  const auto target = FlagSet{}.SetTob().SetSnapshot().SetMaybeBadBook();
  ASSERT_EQ(ToString(target), "TOB | SNAPSHOT | MAYBE_BAD_BOOK (100)");
}

TEST(FlagSetTests, ToStringReservedSet) {
  constexpr FlagSet kTarget{255};
  ASSERT_EQ(ToString(kTarget),
            "LAST | TOB | SNAPSHOT | MBP | BAD_TS_RECV | MAYBE_BAD_BOOK | "
            "PUBLISHER_SPECIFIC (255)");
}

TEST(FlagSetTests, ConstantBitFieldEquivalence) {
  EXPECT_EQ(FlagSet::kLast, FlagSet{}.SetLast().Raw());
  EXPECT_EQ(FlagSet::kTob, FlagSet{}.SetTob().Raw());
  EXPECT_EQ(FlagSet::kSnapshot, FlagSet{}.SetSnapshot().Raw());
  EXPECT_EQ(FlagSet::kMbp, FlagSet{}.SetMbp().Raw());
  EXPECT_EQ(FlagSet::kBadTsRecv, FlagSet{}.SetBadTsRecv().Raw());
  EXPECT_EQ(FlagSet::kMaybeBadBook, FlagSet{}.SetMaybeBadBook().Raw());
}
}  // namespace databento::tests
