#include <gtest/gtest.h>
#include <stdexcept>
#include <cstdlib>

#include "databento/enums.hpp"
#include "databento/historical.hpp"

namespace {
TEST(HistoricalBuilderTests, TestBasic) {
  constexpr auto kKey = "SECRET";
  
  const auto client = databento::HistoricalBuilder()
                          .key(kKey)
                          .gateway(databento::HistoricalGateway::Bo1)
                          .Build();
  EXPECT_EQ(client.key(), kKey);
  EXPECT_EQ(client.gateway(), "https://hist.databento.com");
}

TEST(HistoricalBuilderTests, TestMissingKey) {
  ASSERT_THROW(databento::HistoricalBuilder().Build(), std::logic_error);
}

TEST(HistoricalBuilderTests, TestkeyFromEnv) {
  constexpr auto kKey = "SECRET_KEY";
  ASSERT_EQ(::setenv("DATABENTO_API_KEY", kKey, 1), 0) << "Failed to set environment variable";
  const auto client = databento::HistoricalBuilder().keyFromEnv().Build();
  EXPECT_EQ(client.key(), kKey);
  EXPECT_EQ(client.gateway(), "https://hist.databento.com");
  // unsetting prevents this test from affecting others
  ASSERT_EQ(::unsetenv("DATABENTO_API_KEY"), 0) << "Failed to unset environment variable";
}

TEST(HistoricalBuilderTests, TestkeyFromEnvMissing) {
  ASSERT_THROW(databento::HistoricalBuilder().keyFromEnv().Build(), std::runtime_error);
}
}  // namespace
