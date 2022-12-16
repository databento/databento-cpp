#include <cstdlib>
#include <databento/live.hpp>

#include "databento/live_blocking.hpp"

int main() {
  // auto client =
  // databento::LiveBuilder{}.SetKeyFromEnv().Build<LiveBlocking>();
  databento::LiveBlocking client{std::getenv("DATABENTO_API_KEY"), "127.0.0.1",
                                 9000, false};
  return 0;
}
