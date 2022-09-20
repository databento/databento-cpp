#include <gtest/gtest.h>

#include "mock/mock_server.hpp"

namespace databento {
namespace test {
static auto constexpr kApiKey = "HTTP_SECRET";

class HttpClientTests : public testing::Test {
 protected:
  mock::MockServer server_{kApiKey};
};
}  // namespace test
}  // namespace databento
