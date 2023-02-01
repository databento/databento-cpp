#include <gtest/gtest.h>

#include "databento/constants.hpp"   // dataset
#include "databento/exceptions.hpp"  // Exception, InvalidArgumentError
#include "databento/live.hpp"
#include "mock/mock_tcp_server.hpp"  // MockTcpServer

namespace {
constexpr auto kKey = "32-character-with-lots-of-filler";
}

namespace databento {
namespace test {
TEST(LiveBuilderTests, TestBasic) {
  GTEST_SKIP();
  const auto client = databento::LiveBuilder()
                          .SetKey(kKey)
                          .SetDataset(databento::dataset::kXnasItch)
                          .BuildBlocking();
  EXPECT_EQ(client.Key(), kKey);
  // TODO(cg): update when live gateway URLs are known
  // EXPECT_EQ(client.Gateway(), "https://xnas_itch.lsg.databento.com");
}

TEST(LiveBuilderTests, TestShortKey) {
  constexpr auto kKey = "SHORT_SECRET";
  ASSERT_THROW(databento::LiveBuilder().SetKey(kKey), InvalidArgumentError);
}

TEST(LiveBuilderTests, TestMissingKey) {
  ASSERT_THROW(databento::LiveBuilder().BuildThreaded(), Exception);
}

TEST(LiveBuilderTests, TestSetKeyFromEnv) {
  // TODO(cg): enable when there's a server to connect to
  GTEST_SKIP();

  constexpr auto kKey = "SECRET_KEY";
  ASSERT_EQ(::setenv("DATABENTO_API_KEY", kKey, 1), 0)
      << "Failed to set environment variable";
  const auto client = databento::LiveBuilder().SetKeyFromEnv().BuildBlocking();
  EXPECT_EQ(client.Key(), kKey);
  // EXPECT_EQ(client.Gateway(), "https://hist.databento.com");
  // unsetting prevents this test from affecting others
  ASSERT_EQ(::unsetenv("DATABENTO_API_KEY"), 0)
      << "Failed to unset environment variable";
}

TEST(LiveBuilderTests, TestSetKeyFromEnvMissing) {
  ASSERT_THROW(databento::LiveBuilder().SetKeyFromEnv().BuildThreaded(),
               Exception);
}
}  // namespace test
}  // namespace databento
