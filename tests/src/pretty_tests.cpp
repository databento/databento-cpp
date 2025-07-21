#include <gtest/gtest.h>

#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include "databento/constants.hpp"
#include "databento/datetime.hpp"
#include "databento/pretty.hpp"

namespace databento::pretty::tests {

TEST(PrettyTests, TestPrettyPx) {
  std::ostringstream ss;
  std::vector<std::pair<int64_t, std::string>> cases{{-100'000, "-0.000100000"},
                                                     {32'500'000'000, "32.500000000"},
                                                     {101'005'000'000, "101.005000000"},
                                                     {0, "0.000000000"},
                                                     {kUndefPrice, "UNDEF_PRICE"}};
  for (const auto& [num, exp] : cases) {
    ss << Px{num};
    ASSERT_EQ(ss.str(), exp);
    ss.str("");
  }
}
TEST(PrettyTests, TestPrecision) {
  std::ostringstream ss;
  std::vector<std::tuple<int64_t, int, std::string>> cases{
      {32'500'000'000, 3, "32.500"},
      {101'005'000'000, 5, "101.00500"},
      {75'000'000, 5, "0.07500"},
      {32'123'456'789, 2, "32.12"}};
  for (const auto& [num, precision, exp] : cases) {
    ss << std::setprecision(precision) << Px{num};
    ASSERT_EQ(ss.str(), exp);
    ss.str("");
  }
}

TEST(PrettyTests, TestDefaultFill) {
  std::ostringstream ss;
  std::vector<std::tuple<int64_t, int, int, std::string, std::string>> cases{
      {32'500'000'000, 4, 3, "32.500", "32.500"},
      {32'500'000'000, 8, 3, "  32.500", "32.500  "},
      {101'005'000'000, 10, 5, " 101.00500", "101.00500 "},
      {75'000'000, 13, 5, "      0.07500", "0.07500      "},
      {32'123'456'789, 7, 2, "  32.12", "32.12  "},
      {32'123'456'789, 16, 5, "        32.12345", "32.12345        "}};
  for (const auto& [num, width, precision, exp_right, exp_left] : cases) {
    // Default
    ss << std::setw(width) << std::setprecision(precision) << Px{num};
    ASSERT_EQ(ss.str(), exp_right);
    ss.str("");
    // Left
    ss << std::setw(width) << std::left << std::setprecision(precision) << Px{num};
    ASSERT_EQ(ss.str(), exp_left);
    ss.str("");
    // Right
    ss << std::setw(width) << std::right << std::setprecision(precision) << Px{num};
    ASSERT_EQ(ss.str(), exp_right);
    ss.str("");
  }
}

TEST(PrettyTests, TestZeroFill) {
  std::ostringstream ss;
  std::vector<std::tuple<int64_t, int, int, std::string, std::string>> cases{
      {32'500'000'000, 4, 3, "32.500", "32.500"},
      {32'500'000'000, 8, 3, "0032.500", "32.50000"},
      {101'005'000'000, 10, 5, "0101.00500", "101.005000"},
      {75'000'000, 13, 5, "0000000.07500", "0.07500000000"},
      {32'123'456'789, 7, 2, "0032.12", "32.1200"},
      {32'123'456'789, 16, 4, "00000000032.1234", "32.1234000000000"},
  };
  for (const auto& [num, width, precision, exp_right, exp_left] : cases) {
    // Default
    ss << std::setw(width) << std::setfill('0') << std::setprecision(precision)
       << Px{num};
    ASSERT_EQ(ss.str(), exp_right);
    ss.str("");
    // Left
    ss << std::setw(width) << std::left << std::setfill('0')
       << std::setprecision(precision) << Px{num};
    ASSERT_EQ(ss.str(), exp_left);
    ss.str("");
    // Right
    ss << std::setw(width) << std::right << std::setfill('0')
       << std::setprecision(precision) << Px{num};
    ASSERT_EQ(ss.str(), exp_right);
    ss.str("");
  }
}

TEST(PrettyTests, TestPrettyTs) {
  std::ostringstream ss;
  std::vector<std::pair<int64_t, std::string>> cases{
      {0, "1970-01-01T00:00:00.000000000Z"},
      {1, "1970-01-01T00:00:00.000000001Z"},
      {1622838300000000000, "2021-06-04T20:25:00.000000000Z"},
      {kUndefTimestamp - 1, "2554-07-21T23:34:33.709551614Z"},
      {kUndefTimestamp, "UNDEF_TIMESTAMP"}};
  for (const auto& [num, exp] : cases) {
    ss << Ts{UnixNanos{std::chrono::nanoseconds{num}}};
    ASSERT_EQ(ss.str(), exp);
    ss.str("");
  }
}
}  // namespace databento::pretty::tests
