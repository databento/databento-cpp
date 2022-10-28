#include <gtest/gtest.h>

#include "databento/symbology.hpp"

namespace databento {
namespace test {
TEST(SymbologyTests, TestSymbologyResolutionToString) {
  SymbologyResolution target{
      {
          {"ESM2", {{"2022-06-01", "2022-06-17", "12344"}}},
          {"ESU2", {{"2022-06-01", "2022-07-01", "12345"}}},
      },
      {"ESM2"},
      {"EEES"},
  };
  const auto res = ToString(target);
  // Try both orders because it depends on hash implementation
  if (res != R"(SymbologyResolution {
    mappings = {
        { "ESU2", { StrMappingInterval { start_date = "2022-06-01", end_date = "2022-07-01", symbol = "12345" } } },
        { "ESM2", { StrMappingInterval { start_date = "2022-06-01", end_date = "2022-06-17", symbol = "12344" } } }
    },
    partial = { "ESM2" },
    not_found = { "EEES" }
})" && res != R"(SymbologyResolution {
    mappings = {
        { "ESM2", { StrMappingInterval { start_date = "2022-06-01", end_date = "2022-06-17", symbol = "12344" } } },
        { "ESU2", { StrMappingInterval { start_date = "2022-06-01", end_date = "2022-07-01", symbol = "12345" } } }
    },
    partial = { "ESM2" },
    not_found = { "EEES" }
})") {
    FAIL() << res;
  }
}
}  // namespace test
}  // namespace databento
