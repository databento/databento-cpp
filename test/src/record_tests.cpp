#include <gtest/gtest.h>

#include <chrono>
#include <cstddef>

#include "databento/datetime.hpp"  // TimeDeltaNanos, UnixNanos
#include "databento/record.hpp"

namespace databento {
namespace test {
TEST(RecordTests, TestMbp10MsgToString) {
  Mbp10Msg target{RecordHeader{50, RType::Mbp10, 1, 1, UnixNanos{}},
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

TEST(RecordTests, TestInstrumentDefMsgToString) {
  const InstrumentDefMsg target{
      RecordHeader{sizeof(InstrumentDefMsg) / 4, RType::InstrumentDef, 1, 1,
                   UnixNanos{}},
      UnixNanos{},
      1,
      2,
      3,
      4,
      5,
      6,
      7,
      8,
      9,
      10,
      11,
      12,
      13,
      14,
      15,
      16,
      17,
      18,
      19,
      20,
      21,
      22,
      23,
      24,
      25,
      26,
      27,
      28,
      29,
      30,
      31,
      32,
      {'U', 'S', 'D'},
      {'U', 'S', 'D'},
      {'A'},
      {'T', 'E', 'S', 'T'},
      {'G', 'R', 'O', 'U', 'P'},
      {'C', 'M', 'E'},
      {'A'},
      {'B'},
      {'C'},
      {'D'},
      {},
      {},
      'E',
      33,
      34,
      35,
      36,
      37,
      38,
      'F',
      39,
      40,
      41,
      'G',
      42,
      43,
      44,
      {}};
  const auto res = ToString(target);
  ASSERT_EQ(res, R"(InstrumentDefMsg {
    hd = RecordHeader { length = 90, rtype = 19, publisher_id = 1, product_id = 1, ts_event = 0 },
    ts_recv = 0,
    min_price_increment = 1,
    display_factor = 2,
    expiration = 3,
    activation = 4,
    high_limit_price = 5,
    low_limit_price = 6,
    max_price_variation = 7,
    trading_reference_price = 8,
    unit_of_measure_qty = 9,
    min_price_increment_amount = 10,
    price_ratio = 11,
    inst_attrib_value = 12,
    underlying_id = 13,
    cleared_volume = 14,
    market_depth_implied = 15,
    market_depth = 16,
    market_segment_id = 17,
    max_trade_vol = 18,
    min_lot_size = 19,
    min_lot_size_block = 20,
    min_lot_size_round_lot = 21,
    min_trade_vol = 22,
    open_interest_qty = 23,
    contract_multiplier = 24,
    decay_quantity = 25,
    original_contract_size = 26,
    related_security_id = 27,
    trading_reference_date = 28,
    appl_id = 29,
    maturity_year = 30,
    decay_start_date = 31,
    channel_id = 32,
    currency = "USD",
    settl_currency = "USD",
    secsubtype = "A",
    symbol = "TEST",
    group = "GROUP",
    exchange = "CME",
    asset = "A",
    cfi = "B",
    security_type = "C",
    unit_of_measure = "D",
    underlying = "",
    related = "",
    match_algorithm = 'E',
    md_security_trading_status = 33,
    main_fraction = 34,
    price_display_format = 35,
    settl_price_type = 36,
    sub_fraction = 37,
    underlying_product = 38,
    security_update_action = 'F',
    maturity_month = 39,
    maturity_day = 40,
    maturity_week = 41,
    user_defined_instrument = 'G',
    contract_multiplier_unit = 42,
    flow_schedule_type = 43,
    tick_rule = 44
})");
}
}  // namespace test
}  // namespace databento
