#include <gtest/gtest.h>

#include "databento/constants.hpp"   // dataset
#include "databento/exceptions.hpp"  // Exception, InvalidArgumentError
#include "databento/live.hpp"
#include "mock/mock_tcp_server.hpp"  // MockTcpServer

namespace {
constexpr auto kKey = "32-character-with-lots-of-filler";
}

namespace databento::tests {
TEST(LiveBuilderTests, TestShortKey) {
  constexpr auto kKey = "SHORT_SECRET";
  ASSERT_THROW(databento::LiveBuilder().SetKey(kKey), InvalidArgumentError);
}

TEST(LiveBuilderTests, TestMissingKey) {
  ASSERT_THROW(databento::LiveBuilder()
                   .SetDataset(databento::dataset::kXnasItch)
                   .BuildThreaded(),
               Exception);
}

TEST(LiveBuilderTests, TestMissingDataset) {
  ASSERT_THROW(databento::LiveBuilder().SetKey(kKey).BuildThreaded(), Exception);
}

TEST(LiveBuilderTests, TestSetKeyFromEnvMissing) {
  ASSERT_THROW(databento::LiveBuilder().SetKeyFromEnv().BuildThreaded(), Exception);
}
}  // namespace databento::tests
