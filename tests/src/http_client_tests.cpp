#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "databento/detail/http_client.hpp"
#include "databento/log.hpp"
#include "mock/mock_http_server.hpp"
#include "mock/mock_log_receiver.hpp"

namespace databento::detail::tests {
class HttpClientTests : public ::testing::Test {
 protected:
  static constexpr auto kApiKey = "HIST_SECRET";

  databento::tests::mock::MockHttpServer mock_server_{kApiKey};
};

TEST_F(HttpClientTests, TestLogWarnings) {
  const nlohmann::json warnings{"DeprecationWarning: stype product_id is deprecated",
                                "Warning: Large request"};
  mock_server_.MockGetJson("/warn", {}, {}, warnings);
  const auto port = mock_server_.ListenOnThread();
  databento::tests::mock::MockLogReceiver mock_logger{
      [](auto call_count, databento::LogLevel level, const std::string& msg) {
        EXPECT_EQ(level, LogLevel::Warning);
        if (call_count == 0) {
          EXPECT_THAT(msg,
                      testing::EndsWith(
                          "Server DeprecationWarning: stype product_id is deprecated"));
        } else {
          EXPECT_THAT(msg, testing::EndsWith("Server Warning: Large request"));
        }
      }};
  HttpClient target{&mock_logger, kApiKey, "localhost",
                    static_cast<std::uint16_t>(port)};
  target.GetJson("/warn", {});
  ASSERT_EQ(mock_logger.CallCount(), 2);
}
}  // namespace databento::detail::tests
