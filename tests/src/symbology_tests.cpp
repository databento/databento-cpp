#include <date/date.h>
#include <gtest/gtest.h>

#include "databento/enums.hpp"
#include "databento/symbology.hpp"

namespace databento::tests {
TEST(SymbologyTests, TestSymbologyResolutionToString) {
  const SymbologyResolution target{
      {
          {"ESM2", {{date::year{2022} / 6 / 1, date::year{2022} / 6 / 17, "12344"}}},
          {"ESU2", {{date::year{2022} / 6 / 1, date::year{2022} / 7 / 1, "12345"}}},
      },
      {"ESM2"},
      {"EEES"},
      SType::RawSymbol,
      SType::InstrumentId};
  const auto res = ToString(target);
  // Try both orders because it depends on hash implementation
  if (res != R"(SymbologyResolution {
    mappings = {
        { "ESU2", { MappingInterval { start_date = 2022-06-01, end_date = 2022-07-01, symbol = "12345" } } },
        { "ESM2", { MappingInterval { start_date = 2022-06-01, end_date = 2022-06-17, symbol = "12344" } } }
    },
    partial = { "ESM2" },
    not_found = { "EEES" },
    stype_in = raw_symbol,
    stype_out = instrument_id
})" && res != R"(SymbologyResolution {
    mappings = {
        { "ESM2", { MappingInterval { start_date = 2022-06-01, end_date = 2022-06-17, symbol = "12344" } } },
        { "ESU2", { MappingInterval { start_date = 2022-06-01, end_date = 2022-07-01, symbol = "12345" } } }
    },
    partial = { "ESM2" },
    not_found = { "EEES" },
    stype_in = raw_symbol,
    stype_out = instrument_id
})") {
    FAIL() << res;
  }
}
}  // namespace databento::tests
