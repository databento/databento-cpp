#include "databento/constants.hpp"
#include "databento/live.hpp"
#include "databento/live_blocking.hpp"

int main() {
  auto client = databento::LiveBuilder{}
                    .SetKeyFromEnv()
                    .SetDataset(databento::dataset::kXnasItch)
                    .BuildBlocking();
  return 0;
}
