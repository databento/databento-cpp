#include <gtest/gtest.h>

#include <chrono>
#include <cstdint>

#include "databento/constants.hpp"
#include "databento/datetime.hpp"  // TimeDeltaNanos, UnixNanos
#include "databento/enums.hpp"
#include "databento/publishers.hpp"
#include "databento/record.hpp"

namespace databento::tests {
TEST(RecordTests, TestRecordToString) {
  TradeMsg target{RecordHeader{sizeof(TradeMsg) / RecordHeader::kLengthMultiplier,
                               RType::Mbp0,
                               static_cast<std::uint16_t>(Publisher::OpraPillarEdgo),
                               1,
                               {}},
                  55000000000,
                  500,
                  Action::Add,
                  Side::Bid,
                  {},
                  0,
                  {},
                  {},
                  126239};
  const Record rec{&target.hd};
  EXPECT_EQ(ToString(rec),
            "Record { ptr = RecordHeader { length = 12, rtype = mbp-0, publisher_id "
            "= 24, instrument_id = 1, ts_event = 1970-01-01T00:00:00.000000000Z } }");
}
TEST(RecordTests, TestPublisher) {
  const TradeMsg target{
      RecordHeader{sizeof(TradeMsg) / RecordHeader::kLengthMultiplier,
                   RType::Mbp0,
                   static_cast<std::uint16_t>(Publisher::OpraPillarEdgo),
                   1,
                   {}},
      55000000000,
      500,
      Action::Add,
      Side::Bid,
      {},
      0,
      {},
      {},
      126239};
  EXPECT_EQ(target.hd.Publisher(), Publisher::OpraPillarEdgo);
  EXPECT_EQ(PublisherVenue(target.hd.Publisher()), Venue::Edgo);
  EXPECT_EQ(PublisherDataset(target.hd.Publisher()), Dataset::OpraPillar);
}
TEST(RecordTests, TestMbp10MsgToString) {
  Mbp10Msg target{RecordHeader{sizeof(Mbp10Msg) / RecordHeader::kLengthMultiplier,
                               RType::Mbp10, 1, 1, UnixNanos{}},
                  100000000,
                  10,
                  Action::Add,
                  Side::Bid,
                  {},
                  0,
                  UnixNanos{std::chrono::nanoseconds{1'696'957'072'000'020'500}},
                  TimeDeltaNanos{100},
                  50,
                  {}};
  for (std::uint32_t i = 0; i < 10; ++i) {
    target.levels[i].ask_ct = i;
    target.levels[i].bid_ct = i * 2;
    target.levels[i].ask_sz = i * 3;
    target.levels[i].bid_sz = i * 4;
    target.levels[i].bid_px = static_cast<int64_t>(i) * 5;
    target.levels[i].ask_px = static_cast<int64_t>(i) * 6;
  }
  const auto res = ToString(target);
  ASSERT_EQ(res, R"(Mbp10Msg {
    hd = RecordHeader { length = 92, rtype = mbp-10, publisher_id = 1, instrument_id = 1, ts_event = 1970-01-01T00:00:00.000000000Z },
    price = 0.100000000,
    size = 10,
    action = Add,
    side = Bid,
    flags = 0,
    depth = 0,
    ts_recv = 2023-10-10T16:57:52.000020500Z,
    ts_in_delta = 100,
    sequence = 50,
    levels = {
        BidAskPair { bid_px = 0.000000000, ask_px = 0.000000000, bid_sz = 0, ask_sz = 0, bid_ct = 0, ask_ct = 0 },
        BidAskPair { bid_px = 0.000000005, ask_px = 0.000000006, bid_sz = 4, ask_sz = 3, bid_ct = 2, ask_ct = 1 },
        BidAskPair { bid_px = 0.000000010, ask_px = 0.000000012, bid_sz = 8, ask_sz = 6, bid_ct = 4, ask_ct = 2 },
        BidAskPair { bid_px = 0.000000015, ask_px = 0.000000018, bid_sz = 12, ask_sz = 9, bid_ct = 6, ask_ct = 3 },
        BidAskPair { bid_px = 0.000000020, ask_px = 0.000000024, bid_sz = 16, ask_sz = 12, bid_ct = 8, ask_ct = 4 },
        BidAskPair { bid_px = 0.000000025, ask_px = 0.000000030, bid_sz = 20, ask_sz = 15, bid_ct = 10, ask_ct = 5 },
        BidAskPair { bid_px = 0.000000030, ask_px = 0.000000036, bid_sz = 24, ask_sz = 18, bid_ct = 12, ask_ct = 6 },
        BidAskPair { bid_px = 0.000000035, ask_px = 0.000000042, bid_sz = 28, ask_sz = 21, bid_ct = 14, ask_ct = 7 },
        BidAskPair { bid_px = 0.000000040, ask_px = 0.000000048, bid_sz = 32, ask_sz = 24, bid_ct = 16, ask_ct = 8 },
        BidAskPair { bid_px = 0.000000045, ask_px = 0.000000054, bid_sz = 36, ask_sz = 27, bid_ct = 18, ask_ct = 9 }
    }
})");
}

TEST(RecordTests, TestInstrumentDefMsgToString) {
  const InstrumentDefMsg target{
      RecordHeader{sizeof(InstrumentDefMsg) / RecordHeader::kLengthMultiplier,
                   RType::InstrumentDef, 1, 1, UnixNanos{}},
      UnixNanos{},
      1,
      2,
      UnixNanos{},
      UnixNanos{},
      5,
      6,
      7,
      8,
      9,
      10,
      kUndefPrice,
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
      33,
      34,
      35,
      36,
      37,
      38,
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
      {'E', 'S', 'M', '5'},
      InstrumentClass::Future,
      MatchAlgorithm::Fifo,
      33,
      34,
      35,
      36,
      SecurityUpdateAction::Add,
      39,
      40,
      41,
      UserDefinedInstrument::No,
      42,
      43,
      44,
      InstrumentClass::CommoditySpot,
      Side::Bid};
  const auto res = ToString(target);
  ASSERT_EQ(res, R"(InstrumentDefMsg {
    hd = RecordHeader { length = 130, rtype = instrument-def, publisher_id = 1, instrument_id = 1, ts_event = 1970-01-01T00:00:00.000000000Z },
    ts_recv = 1970-01-01T00:00:00.000000000Z,
    min_price_increment = 0.000000001,
    display_factor = 0.000000002,
    expiration = 1970-01-01T00:00:00.000000000Z,
    activation = 1970-01-01T00:00:00.000000000Z,
    high_limit_price = 0.000000005,
    low_limit_price = 0.000000006,
    max_price_variation = 0.000000007,
    unit_of_measure_qty = 0.000000008,
    min_price_increment_amount = 0.000000009,
    price_ratio = 0.000000010,
    strike_price = UNDEF_PRICE,
    raw_instrument_id = 11,
    leg_price = 0.000000012,
    leg_delta = 0.000000013,
    inst_attrib_value = 14,
    underlying_id = 15,
    market_depth_implied = 16,
    market_depth = 17,
    market_segment_id = 18,
    max_trade_vol = 19,
    min_lot_size = 20,
    min_lot_size_block = 21,
    min_lot_size_round_lot = 22,
    min_trade_vol = 23,
    contract_multiplier = 24,
    decay_quantity = 25,
    original_contract_size = 26,
    leg_instrument_id = 27,
    leg_ratio_price_numerator = 28,
    leg_ratio_price_denominator = 29,
    leg_ratio_qty_numerator = 30,
    leg_ratio_qty_denominator = 31,
    leg_underlying_id = 32,
    appl_id = 33,
    maturity_year = 34,
    decay_start_date = 35,
    channel_id = 36,
    leg_count = 37,
    leg_index = 38,
    currency = "USD",
    settl_currency = "USD",
    secsubtype = "A",
    raw_symbol = "TEST",
    group = "GROUP",
    exchange = "CME",
    asset = "A",
    cfi = "B",
    security_type = "C",
    unit_of_measure = "D",
    underlying = "",
    strike_price_currency = "",
    leg_raw_symbol = "ESM5",
    instrument_class = Future,
    match_algorithm = Fifo,
    main_fraction = 33,
    price_display_format = 34,
    sub_fraction = 35,
    underlying_product = 36,
    security_update_action = Add,
    maturity_month = 39,
    maturity_day = 40,
    maturity_week = 41,
    user_defined_instrument = No,
    contract_multiplier_unit = 42,
    flow_schedule_type = 43,
    tick_rule = 44,
    leg_instrument_class = CommoditySpot,
    leg_side = Bid
})");
}

TEST(RecordTests, TestImbalanceMsgToString) {
  const ImbalanceMsg target{
      RecordHeader{sizeof(ImbalanceMsg) / RecordHeader::kLengthMultiplier,
                   RType::Imbalance, 1, 1, UnixNanos{}},
      UnixNanos{},
      1,
      UnixNanos{std::chrono::nanoseconds{kUndefTimestamp}},
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
      'A',
      Side::Ask,
      15,
      16,
      17,
      Side::None,
      'N',
      {}};
  const auto res = ToString(target);
  ASSERT_EQ(res, R"(ImbalanceMsg {
    hd = RecordHeader { length = 28, rtype = imbalance, publisher_id = 1, instrument_id = 1, ts_event = 1970-01-01T00:00:00.000000000Z },
    ts_recv = 1970-01-01T00:00:00.000000000Z,
    ref_price = 0.000000001,
    auction_time = UNDEF_TIMESTAMP,
    cont_book_clr_price = 0.000000003,
    auct_interest_clr_price = 0.000000004,
    ssr_filling_price = 0.000000005,
    ind_match_price = 0.000000006,
    upper_collar = 0.000000007,
    lower_collar = 0.000000008,
    paired_qty = 9,
    total_imbalance_qty = 10,
    market_imbalance_qty = 11,
    unpaired_qty = 12,
    auction_type = 'A',
    side = Ask,
    auction_status = 15,
    freeze_status = 16,
    num_extensions = 17,
    unpaired_side = None,
    significant_imbalance = 'N'
})");
}
}  // namespace databento::tests
