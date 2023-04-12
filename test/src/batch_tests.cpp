#include <gtest/gtest.h>

#include "databento/batch.hpp"
#include "databento/constants.hpp"
#include "databento/enums.hpp"

namespace databento {
namespace test {
TEST(BatchTests, TestBatchJobToString) {
  const BatchJob target{"aNiD",
                        "USER",
                        "57db",
                        12.39,
                        dataset::kXnasItch,
                        {"CL.FUT"},
                        SType::Smart,
                        SType::ProductId,
                        Schema::Ohlcv1M,
                        {},
                        {},
                        {},
                        Encoding::Dbn,
                        Compression::None,
                        SplitDuration::Week,
                        {},
                        false,
                        Packaging::Tar,
                        Delivery::Download,
                        10250,
                        35000000,
                        20000000,
                        0,
                        JobState::Processing,
                        {},
                        {},
                        {},
                        {},
                        {}};
  const auto res = ToString(target);
  ASSERT_EQ(res, R"(BatchJob {
    id = "aNiD",
    user_id = "USER",
    bill_id = "57db",
    cost_usd = 12.39,
    dataset = "XNAS.ITCH",
    symbols = { "CL.FUT" },
    stype_in = smart,
    stype_out = product_id,
    schema = ohlcv-1m,
    start = "",
    end = "",
    limit = 0,
    encoding = dbn,
    compression = none,
    split_duration = week,
    split_size = 0,
    split_symbols = false,
    packaging = tar,
    delivery = download,
    record_count = 10250,
    billed_size = 35000000,
    actual_size = 20000000,
    package_size = 0,
    state = processing,
    ts_received = "",
    ts_queued = "",
    ts_process_start = "",
    ts_process_done = "",
    ts_expiration = ""
})");
}
}  // namespace test
}  // namespace databento
