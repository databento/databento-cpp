#include <gtest/gtest.h>

#include "databento/batch.hpp"
#include "databento/constants.hpp"
#include "databento/enums.hpp"

namespace databento::tests {
TEST(BatchTests, TestBatchJobToString) {
  const BatchJob target{"aNiD",
                        "USER",
                        12.39,
                        dataset::kXnasItch,
                        {"CL.FUT"},
                        SType::Parent,
                        SType::InstrumentId,
                        Schema::Ohlcv1M,
                        {},
                        {},
                        {},
                        Encoding::Dbn,
                        Compression::None,
                        true,
                        false,
                        true,
                        SplitDuration::Week,
                        {},
                        false,
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
    cost_usd = 12.39,
    dataset = "XNAS.ITCH",
    symbols = { "CL.FUT" },
    stype_in = parent,
    stype_out = instrument_id,
    schema = ohlcv-1m,
    start = "",
    end = "",
    limit = 0,
    encoding = dbn,
    compression = none,
    pretty_px = true,
    pretty_ts = false,
    map_symbols = true,
    split_duration = week,
    split_size = 0,
    split_symbols = false,
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
}  // namespace databento::tests
