#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <iostream>

#include "databento/detail/http_client.hpp"
#include "databento/log.hpp"
#include "mock/mock_http_server.hpp"

namespace databento::detail::tests {
class HttpClientTests : public ::testing::Test {
 protected:
  static constexpr auto kApiKey = "HIST_SECRET";

  databento::tests::mock::MockHttpServer mock_server_{kApiKey};
};

TEST_F(HttpClientTests, TestLogWarnings) {
  const nlohmann::json warnings{
      "DeprecationWarning: stype product_id is deprecated",
      "Warning: Large request"};
  mock_server_.MockGetJson("/warn", {}, {}, warnings);
  const auto port = mock_server_.ListenOnThread();
  HttpClient target{ILogReceiver::Default(), kApiKey, "localhost",
                    static_cast<std::uint16_t>(port)};
  testing::internal::CaptureStderr();
  target.GetJson("/warn", {});
  std::clog.flush();
  const std::string output = testing::internal::GetCapturedStderr();
  EXPECT_EQ(std::count(output.begin(), output.end(), '\n'), 2);
  EXPECT_NE(
      output.find("Server DeprecationWarning: stype product_id is deprecated"),
      std::string::npos);
  EXPECT_NE(output.find("Server Warning: Large request"), std::string::npos);
}
}  // namespace databento::detail::tests
