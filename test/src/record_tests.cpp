#include <gtest/gtest.h>

#include <chrono>
#include <cstddef>

#include "databento/datetime.hpp"
#include "databento/record.hpp"

namespace databento {
namespace test {
TEST(RecordTests, TestMbp10MsgToString) {
  Mbp10Msg target{RecordHeader{50, Mbp10Msg::kTypeId, 1, 1, UnixNanos{}},
                  100000000,
                  10,
                  'A',
                  'B',
                  {},
                  0,
                  UnixNanos{},
                  TimeDeltaNanos{100},
                  50,
                  {}};
  for (std::uint32_t i = 0; i < 10; ++i) {
    target.booklevel[i].ask_ct = i;
    target.booklevel[i].bid_ct = i * 2;
    target.booklevel[i].ask_sz = i * 3;
    target.booklevel[i].bid_sz = i * 4;
    target.booklevel[i].bid_px = static_cast<int64_t>(i) * 5;
    target.booklevel[i].ask_px = static_cast<int64_t>(i) * 6;
  }
  const auto res = ToString(target);
  ASSERT_EQ(res, R"(Mbp10Msg {
    hd = RecordHeader { length = 50, rtype = 10, publisher_id = 1, product_id = 1, ts_event = 0 },
    price = 100000000,
    size = 10,
    action = 'A',
    side = 'B',
    flags = 0,
    depth = 0,
    ts_recv = 0,
    ts_in_delta = 100,
    sequence = 50,
    booklevel = {
        BidAskPair { bid_px = 0, ask_px = 0, bid_sz = 0, ask_sz = 0, bid_ct = 0, ask_ct = 0 },
        BidAskPair { bid_px = 5, ask_px = 6, bid_sz = 4, ask_sz = 3, bid_ct = 2, ask_ct = 1 },
        BidAskPair { bid_px = 10, ask_px = 12, bid_sz = 8, ask_sz = 6, bid_ct = 4, ask_ct = 2 },
        BidAskPair { bid_px = 15, ask_px = 18, bid_sz = 12, ask_sz = 9, bid_ct = 6, ask_ct = 3 },
        BidAskPair { bid_px = 20, ask_px = 24, bid_sz = 16, ask_sz = 12, bid_ct = 8, ask_ct = 4 },
        BidAskPair { bid_px = 25, ask_px = 30, bid_sz = 20, ask_sz = 15, bid_ct = 10, ask_ct = 5 },
        BidAskPair { bid_px = 30, ask_px = 36, bid_sz = 24, ask_sz = 18, bid_ct = 12, ask_ct = 6 },
        BidAskPair { bid_px = 35, ask_px = 42, bid_sz = 28, ask_sz = 21, bid_ct = 14, ask_ct = 7 },
        BidAskPair { bid_px = 40, ask_px = 48, bid_sz = 32, ask_sz = 24, bid_ct = 16, ask_ct = 8 },
        BidAskPair { bid_px = 45, ask_px = 54, bid_sz = 36, ask_sz = 27, bid_ct = 18, ask_ct = 9 }
    }
})");
}
}  // namespace test
}  // namespace databento
