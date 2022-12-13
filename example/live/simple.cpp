#include <cstdlib>
#include <databento/live.hpp>

int main() {
  // auto client = databento::LiveBuilder{}.SetKeyFromEnv().Build();
  databento::Live client{std::getenv("DATABENTO_API_KEY"), "127.0.0.1", 9000,
                         false};
  return 0;
}
