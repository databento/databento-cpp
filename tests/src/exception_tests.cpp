#include <gtest/gtest.h>

#include <string>

#include "databento/exceptions.hpp"

namespace databento::tests {

TEST(HttpResponseErrorTests, SimpleDetail) {
  const std::string body =
      R"({"detail": "Authorization failed: illegal chars in username."})";
  const HttpResponseError err{"/v0/timeseries.get_range", 400, body};

  EXPECT_EQ(err.StatusCode(), 400);
  EXPECT_EQ(err.RequestPath(), "/v0/timeseries.get_range");
  EXPECT_FALSE(err.Case().has_value());
  ASSERT_TRUE(err.DetailMessage().has_value());
  EXPECT_EQ(*err.DetailMessage(), "Authorization failed: illegal chars in username.");
  EXPECT_FALSE(err.DocsUrl().has_value());
  // Simple-detail responses don't append a `(case: ...)` suffix.
  EXPECT_EQ(std::string{err.what()},
            "Received an error response from request to /v0/timeseries.get_range "
            "with status 400 and body '" +
                body + "'");
}

TEST(HttpResponseErrorTests, BusinessDetail) {
  const std::string body =
      R"({"detail": {)"
      R"("case": "data_start_before_available_start",)"
      R"("message": "start was before the available start.",)"
      R"("status_code": 422,)"
      R"("docs": "https://databento.com/docs/api-reference-historical/metadata/metadata-get-dataset",)"
      R"("payload": {"dataset": "GLBX.MDP3"}}})";
  const HttpResponseError err{"/v0/timeseries.get_range", 422, body};

  EXPECT_EQ(err.StatusCode(), 422);
  ASSERT_TRUE(err.Case().has_value());
  EXPECT_EQ(*err.Case(), "data_start_before_available_start");
  ASSERT_TRUE(err.DetailMessage().has_value());
  EXPECT_EQ(*err.DetailMessage(), "start was before the available start.");
  ASSERT_TRUE(err.DocsUrl().has_value());
  EXPECT_EQ(*err.DocsUrl(),
            "https://databento.com/docs/api-reference-historical/metadata/"
            "metadata-get-dataset");
  // The raw body is still embedded in `what()` for debugging, even though the
  // typed accessors expose the parsed envelope.
  EXPECT_NE(std::string{err.what()}.find(body), std::string::npos);
  EXPECT_NE(std::string{err.what()}.find("(case: data_start_before_available_start)"),
            std::string::npos);
}

TEST(HttpResponseErrorTests, NonJsonBody) {
  const std::string body = "<html>502 Bad Gateway</html>";
  const HttpResponseError err{"/v0/metadata.list_datasets", 502, body};

  EXPECT_EQ(err.StatusCode(), 502);
  EXPECT_NE(std::string{err.what()}.find(body), std::string::npos);
  EXPECT_FALSE(err.Case().has_value());
  EXPECT_FALSE(err.DetailMessage().has_value());
  EXPECT_FALSE(err.DocsUrl().has_value());
}
}  // namespace databento::tests
