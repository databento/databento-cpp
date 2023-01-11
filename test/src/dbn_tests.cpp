#include <gtest/gtest.h>

#include "databento/constants.hpp"
#include "databento/dbn.hpp"

namespace databento {
namespace test {
TEST(DbnTests, TestMetadataToString) {
  const Metadata target{1,
                        dataset::kGlbxMdp3,
                        Schema::Ohlcv1D,
                        {},
                        {},
                        {},
                        73,
                        Compression::None,
                        SType::Native,
                        SType::ProductId,
                        {"NGG3", "NGQ4"},
                        {"ng"},
                        {"nf"},
                        {{"NGG3", {{20220601, 20220701, "3"}}},
                         {"NGQ4", {{20220601, 20220701, "4"}}}}};
  const auto res = ToString(target);
  ASSERT_EQ(res, R"(Metadata {
    version = 1,
    dataset = "GLBX.MDP3",
    schema = ohlcv-1d,
    start = 0,
    end = 0,
    limit = 0,
    record_count = 73,
    compression = none,
    stype_in = native,
    stype_out = product_id,
    symbols = { "NGG3", "NGQ4" },
    partial = { "ng" },
    not_found = { "nf" },
    mappings = {
        SymbolMapping { native = "NGG3", intervals = { MappingInterval { start_date = 20220601, end_date = 20220701, symbol = "3" } } },
        SymbolMapping { native = "NGQ4", intervals = { MappingInterval { start_date = 20220601, end_date = 20220701, symbol = "4" } } }
    }
})");
}
}  // namespace test
}  // namespace databento
