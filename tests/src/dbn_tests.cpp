#include <gtest/gtest.h>

#include <chrono>

#include "databento/constants.hpp"
#include "databento/datetime.hpp"
#include "databento/dbn.hpp"

namespace databento::tests {
TEST(DbnTests, TestMetadataToString) {
  const Metadata target{
      kDbnVersion,
      dataset::kGlbxMdp3,
      Schema::Ohlcv1D,
      UnixNanos{std::chrono::seconds{1696959347}},
      UnixNanos{std::chrono::seconds{1696950000}},
      {},
      SType::RawSymbol,
      SType::InstrumentId,
      false,
      kSymbolCstrLen,
      {"NGG3", "NGQ4"},
      {"ng"},
      {"nf"},
      {{"NGG3", {{date::year{2022} / 6 / 1, date::year{2022} / 7 / 1, "3"}}},
       {"NGQ4", {{date::year{2022} / 6 / 1, date::year{2022} / 7 / 1, "4"}}}}};
  const auto res = ToString(target);
  ASSERT_EQ(res, R"(Metadata {
    version = 3,
    dataset = "GLBX.MDP3",
    schema = ohlcv-1d,
    start = 2023-10-10T17:35:47.000000000Z,
    end = 2023-10-10T15:00:00.000000000Z,
    limit = 0,
    stype_in = raw_symbol,
    stype_out = instrument_id,
    ts_out = false,
    symbol_cstr_len = 71,
    symbols = { "NGG3", "NGQ4" },
    partial = { "ng" },
    not_found = { "nf" },
    mappings = {
        SymbolMapping { raw_symbol = "NGG3", intervals = { MappingInterval { start_date = 2022-06-01, end_date = 2022-07-01, symbol = "3" } } },
        SymbolMapping { raw_symbol = "NGQ4", intervals = { MappingInterval { start_date = 2022-06-01, end_date = 2022-07-01, symbol = "4" } } }
    }
})");
}
}  // namespace databento::tests
