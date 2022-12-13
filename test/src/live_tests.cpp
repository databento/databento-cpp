#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include "databento/exceptions.hpp"  // Exception
#include "databento/live.hpp"
#include "mock/mock_tcp_server.hpp"  // MockTcpServer

namespace databento {
namespace test {
class LiveTests : public testing::Test {
 public:
  static constexpr auto kKey = "32-character-with-lots-of-filler";
  // mock::MockTcpServer mock_server_;
};

TEST_F(LiveTests, TestAuthentication) {
  mock::MockTcpServer mock_server{[](mock::MockTcpServer& self) {
    self.Accept();

    self.SetSend("lsg-test\n");
    self.Write();
    // write challenge separate to test multiple reads to get CRAM challenge
    self.SetSend("cram=t7kNhwj4xqR0QYjzFKtBEG2ec2pXJ4FK\n");
    self.Write();
    std::this_thread::sleep_for(std::chrono::milliseconds{50});
    self.Read();
    {
      const auto received = self.AwaitReceived();
      const auto auth_start = received.find('=') + 1;
      const auto auth =
          received.substr(auth_start, received.find('-') - auth_start);
      EXPECT_EQ(auth.length(), SHA256_DIGEST_LENGTH * 2);
      for (const char c : auth) {
        EXPECT_TRUE((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))
            << "Expected hex character";
      }
      EXPECT_NE(received.find("encoding=dbz"), std::string::npos);
      EXPECT_NE(received.find("ts_out=0"), std::string::npos);
    }
    self.SetSend("success=1|session_id=5|\n");
    self.Write();

    self.Close();
  }};

  Live target_{kKey, "127.0.0.1", mock_server.Port(), false};
}

TEST(LiveBuilderTests, TestBasic) {
  GTEST_SKIP();
  const auto client = databento::LiveBuilder()
                          .SetKey(LiveTests::kKey)
                          .SetGateway(databento::LiveGateway::Origin)
                          .Build();
  EXPECT_EQ(client.Key(), LiveTests::kKey);
  // TODO(cg): update when live gateway URLs are known
  // EXPECT_EQ(client.Gateway(), "https://hist.databento.com");
}

TEST(LiveBuilderTests, TestShortKey) {
  constexpr auto kKey = "SHORT_SECRET";
  ASSERT_THROW(databento::LiveBuilder().SetKey(kKey), InvalidArgumentError);
}

TEST(LiveBuilderTests, TestMissingKey) {
  ASSERT_THROW(databento::LiveBuilder().Build(), Exception);
}

TEST(LiveBuilderTests, TestSetKeyFromEnv) {
  // TODO(cg): enable when there's a server to connect to
  GTEST_SKIP();

  constexpr auto kKey = "SECRET_KEY";
  ASSERT_EQ(::setenv("DATABENTO_API_KEY", kKey, 1), 0)
      << "Failed to set environment variable";
  const auto client = databento::LiveBuilder().SetKeyFromEnv().Build();
  EXPECT_EQ(client.Key(), kKey);
  // EXPECT_EQ(client.Gateway(), "https://hist.databento.com");
  // unsetting prevents this test from affecting others
  ASSERT_EQ(::unsetenv("DATABENTO_API_KEY"), 0)
      << "Failed to unset environment variable";
}

TEST(LiveBuilderTests, TestSetKeyFromEnvMissing) {
  ASSERT_THROW(databento::LiveBuilder().SetKeyFromEnv().Build(), Exception);
}
}  // namespace test
}  // namespace databento
