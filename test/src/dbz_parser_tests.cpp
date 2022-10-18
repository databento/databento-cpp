#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <fstream>  // ifstream
#include <ios>      // streamsize, ios::binary, ios::ate
#include <thread>

#include "databento/dbz.hpp"
#include "databento/dbz_parser.hpp"
#include "databento/enums.hpp"
#include "databento/record.hpp"

namespace databento {
namespace test {
class DbzParserTests : public testing::Test {
 protected:
  DbzParser target_;
  std::thread write_thread_;

  void TearDown() override {
    if (write_thread_.joinable()) {
      write_thread_.join();
    }
  }

  void ReadFromFile(const std::string& file_path) {
    // ifstream with std::uint8_t compiles but doesn't work
    std::ifstream input_file{file_path, std::ios::binary | std::ios::ate};
    const auto size = static_cast<std::size_t>(input_file.tellg());
    input_file.seekg(0, std::ios::beg);
    std::vector<char> buffer(size);
    input_file.read(buffer.data(), static_cast<std::streamsize>(size));
    target_.PassBytes(reinterpret_cast<const std::uint8_t*>(buffer.data()),
                      size);
    target_.EndInput();
  }

  static void AssertMappings(const std::vector<SymbolMapping>& mappings) {
    ASSERT_EQ(mappings.size(), 1);
    const auto& mapping = mappings.at(0);
    EXPECT_EQ(mapping.native, "ESH1");
    ASSERT_EQ(mapping.intervals.size(), 1);
    const auto& interval = mapping.intervals.at(0);
    EXPECT_EQ(interval.symbol, "5482");
    EXPECT_EQ(interval.start_date, 20201228);
    EXPECT_EQ(interval.end_date, 20201229);
  }
};

// Expected data for these tests obtained using the `dbz` CLI tool

TEST_F(DbzParserTests, TestParseMbo) {
  write_thread_ = std::thread(
      [this] { this->ReadFromFile(TEST_BUILD_DIR "/data/test_data.mbo.dbz"); });

  const Metadata metadata = target_.ParseMetadata();
  EXPECT_EQ(metadata.version, 1);
  EXPECT_EQ(metadata.dataset, "GLBX.MDP3");
  EXPECT_EQ(metadata.schema, Schema::Mbo);
  EXPECT_EQ(metadata.start.time_since_epoch().count(), 1609160400000000000);
  EXPECT_EQ(metadata.end.time_since_epoch().count(), 1609200000000000000);
  EXPECT_EQ(metadata.limit, 2);
  EXPECT_EQ(metadata.record_count, 2);
  EXPECT_EQ(metadata.stype_in, SType::Native);
  EXPECT_EQ(metadata.stype_out, SType::ProductId);
  EXPECT_EQ(metadata.symbols, std::vector<std::string>{"ESH1"});
  EXPECT_TRUE(metadata.partial.empty());
  EXPECT_TRUE(metadata.not_found.empty());
  AssertMappings(metadata.mappings);

  const auto record1 = target_.ParseRecord();
  ASSERT_TRUE(record1.holds<TickMsg>());
  const auto& mbo1 = record1.get<TickMsg>();
  EXPECT_EQ(mbo1.hd.publisher_id, 1);
  EXPECT_EQ(mbo1.hd.product_id, 5482);
  EXPECT_EQ(mbo1.hd.ts_event.time_since_epoch().count(), 1609160400000429831);
  EXPECT_EQ(mbo1.order_id, 647784973705);
  EXPECT_EQ(mbo1.price, 3722750000000);
  EXPECT_EQ(mbo1.size, 1);
  EXPECT_EQ(mbo1.flags, -128);
  EXPECT_EQ(mbo1.channel_id, 0);
  EXPECT_EQ(mbo1.action, 'C');
  EXPECT_EQ(mbo1.side, 'A');
  EXPECT_EQ(mbo1.ts_recv.time_since_epoch().count(), 1609160400000704060);
  EXPECT_EQ(mbo1.ts_in_delta.count(), 22993);
  EXPECT_EQ(mbo1.sequence, 1170352);

  const auto record2 = target_.ParseRecord();
  ASSERT_TRUE(record2.holds<TickMsg>());
  const auto& mbo2 = record2.get<TickMsg>();
  EXPECT_EQ(mbo2.hd.publisher_id, 1);
  EXPECT_EQ(mbo2.hd.product_id, 5482);
  EXPECT_EQ(mbo2.hd.ts_event.time_since_epoch().count(), 1609160400000431665);
  EXPECT_EQ(mbo2.order_id, 647784973631);
  EXPECT_EQ(mbo2.price, 3723000000000);
  EXPECT_EQ(mbo2.size, 1);
  EXPECT_EQ(mbo2.flags, -128);
  EXPECT_EQ(mbo2.channel_id, 0);
  EXPECT_EQ(mbo2.action, 'C');
  EXPECT_EQ(mbo2.side, 'A');
  EXPECT_EQ(mbo2.ts_recv.time_since_epoch().count(), 1609160400000711344);
  EXPECT_EQ(mbo2.ts_in_delta.count(), 19621);
  EXPECT_EQ(mbo2.sequence, 1170353);
}

TEST_F(DbzParserTests, TestParseMbp1) {
  write_thread_ = std::thread([this] {
    this->ReadFromFile(TEST_BUILD_DIR "/data/test_data.mbp-1.dbz");
  });

  const Metadata metadata = target_.ParseMetadata();
  EXPECT_EQ(metadata.version, 1);
  EXPECT_EQ(metadata.dataset, "GLBX.MDP3");
  EXPECT_EQ(metadata.schema, Schema::Mbp1);
  EXPECT_EQ(metadata.start.time_since_epoch().count(), 1609160400000000000);
  EXPECT_EQ(metadata.end.time_since_epoch().count(), 1609200000000000000);
  EXPECT_EQ(metadata.limit, 2);
  EXPECT_EQ(metadata.record_count, 2);
  EXPECT_EQ(metadata.stype_in, SType::Native);
  EXPECT_EQ(metadata.stype_out, SType::ProductId);
  EXPECT_EQ(metadata.symbols, std::vector<std::string>{"ESH1"});
  EXPECT_TRUE(metadata.partial.empty());
  EXPECT_TRUE(metadata.not_found.empty());
  AssertMappings(metadata.mappings);

  const auto record1 = target_.ParseRecord();
  ASSERT_TRUE(record1.holds<Mbp1Msg>());
  const auto& mbp1 = record1.get<Mbp1Msg>();
  EXPECT_EQ(mbp1.hd.publisher_id, 1);
  EXPECT_EQ(mbp1.hd.product_id, 5482);
  EXPECT_EQ(mbp1.hd.ts_event.time_since_epoch().count(), 1609160400006001487);
  EXPECT_EQ(mbp1.price, 3720500000000);
  EXPECT_EQ(mbp1.size, 1);
  EXPECT_EQ(mbp1.action, 'A');
  EXPECT_EQ(mbp1.side, 'A');
  EXPECT_EQ(mbp1.flags, -128);
  EXPECT_EQ(mbp1.depth, 0);
  EXPECT_EQ(mbp1.ts_recv.time_since_epoch().count(), 1609160400006136329);
  EXPECT_EQ(mbp1.ts_in_delta.count(), 17214);
  EXPECT_EQ(mbp1.sequence, 1170362);
  EXPECT_EQ(mbp1.booklevel[0].bid_px, 3720250000000);
  EXPECT_EQ(mbp1.booklevel[0].ask_px, 3720500000000);
  EXPECT_EQ(mbp1.booklevel[0].bid_sz, 24);
  EXPECT_EQ(mbp1.booklevel[0].ask_sz, 11);
  EXPECT_EQ(mbp1.booklevel[0].bid_ct, 15);
  EXPECT_EQ(mbp1.booklevel[0].ask_ct, 9);

  const auto record2 = target_.ParseRecord();
  ASSERT_TRUE(record2.holds<Mbp1Msg>());
  const auto& mbp2 = record2.get<Mbp1Msg>();
  EXPECT_EQ(mbp2.hd.publisher_id, 1);
  EXPECT_EQ(mbp2.hd.product_id, 5482);
  EXPECT_EQ(mbp2.hd.ts_event.time_since_epoch().count(), 1609160400006146661);
  EXPECT_EQ(mbp2.price, 3720500000000);
  EXPECT_EQ(mbp2.size, 1);
  EXPECT_EQ(mbp2.action, 'A');
  EXPECT_EQ(mbp2.side, 'A');
  EXPECT_EQ(mbp2.flags, -128);
  EXPECT_EQ(mbp2.depth, 0);
  EXPECT_EQ(mbp2.ts_recv.time_since_epoch().count(), 1609160400006246513);
  EXPECT_EQ(mbp2.ts_in_delta.count(), 18858);
  EXPECT_EQ(mbp2.sequence, 1170364);
  EXPECT_EQ(mbp2.booklevel[0].bid_px, 3720250000000);
  EXPECT_EQ(mbp2.booklevel[0].ask_px, 3720500000000);
  EXPECT_EQ(mbp2.booklevel[0].bid_sz, 24);
  EXPECT_EQ(mbp2.booklevel[0].ask_sz, 12);
  EXPECT_EQ(mbp2.booklevel[0].bid_ct, 15);
  EXPECT_EQ(mbp2.booklevel[0].ask_ct, 10);
}

TEST_F(DbzParserTests, TestParseMbp10) {
  write_thread_ = std::thread([this] {
    this->ReadFromFile(TEST_BUILD_DIR "/data/test_data.mbp-10.dbz");
  });

  const Metadata metadata = target_.ParseMetadata();
  EXPECT_EQ(metadata.version, 1);
  EXPECT_EQ(metadata.dataset, "GLBX.MDP3");
  EXPECT_EQ(metadata.schema, Schema::Mbp10);
  EXPECT_EQ(metadata.start.time_since_epoch().count(), 1609160400000000000);
  EXPECT_EQ(metadata.end.time_since_epoch().count(), 1609200000000000000);
  EXPECT_EQ(metadata.limit, 2);
  EXPECT_EQ(metadata.record_count, 2);
  EXPECT_EQ(metadata.stype_in, SType::Native);
  EXPECT_EQ(metadata.stype_out, SType::ProductId);
  EXPECT_EQ(metadata.symbols, std::vector<std::string>{"ESH1"});
  EXPECT_TRUE(metadata.partial.empty());
  EXPECT_TRUE(metadata.not_found.empty());
  AssertMappings(metadata.mappings);

  const auto record1 = target_.ParseRecord();
  ASSERT_TRUE(record1.holds<Mbp10Msg>());
  const auto& mbp1 = record1.get<Mbp10Msg>();
  EXPECT_EQ(mbp1.hd.publisher_id, 1);
  EXPECT_EQ(mbp1.hd.product_id, 5482);
  EXPECT_EQ(mbp1.hd.ts_event.time_since_epoch().count(), 1609160400000429831);
  EXPECT_EQ(mbp1.price, 3722750000000);
  EXPECT_EQ(mbp1.size, 1);
  EXPECT_EQ(mbp1.action, 'C');
  EXPECT_EQ(mbp1.side, 'A');
  EXPECT_EQ(mbp1.flags, -128);
  EXPECT_EQ(mbp1.depth, 9);
  EXPECT_EQ(mbp1.ts_recv.time_since_epoch().count(), 1609160400000704060);
  EXPECT_EQ(mbp1.ts_in_delta.count(), 22993);
  EXPECT_EQ(mbp1.sequence, 1170352);
  EXPECT_EQ(mbp1.booklevel[0].bid_px, 3720250000000);
  EXPECT_EQ(mbp1.booklevel[0].ask_px, 3720500000000);
  EXPECT_EQ(mbp1.booklevel[0].bid_sz, 24);
  EXPECT_EQ(mbp1.booklevel[0].ask_sz, 10);
  EXPECT_EQ(mbp1.booklevel[0].bid_ct, 15);
  EXPECT_EQ(mbp1.booklevel[0].ask_ct, 8);
  EXPECT_EQ(mbp1.booklevel[1].bid_px, 3720000000000);
  EXPECT_EQ(mbp1.booklevel[1].ask_px, 3720750000000);
  EXPECT_EQ(mbp1.booklevel[1].bid_sz, 31);
  EXPECT_EQ(mbp1.booklevel[1].ask_sz, 34);
  EXPECT_EQ(mbp1.booklevel[1].bid_ct, 18);
  EXPECT_EQ(mbp1.booklevel[1].ask_ct, 24);
  EXPECT_EQ(mbp1.booklevel[2].bid_px, 3719750000000);
  EXPECT_EQ(mbp1.booklevel[2].ask_px, 3721000000000);
  EXPECT_EQ(mbp1.booklevel[2].bid_sz, 32);
  EXPECT_EQ(mbp1.booklevel[2].ask_sz, 39);
  EXPECT_EQ(mbp1.booklevel[2].bid_ct, 23);
  EXPECT_EQ(mbp1.booklevel[2].ask_ct, 25);

  const auto record2 = target_.ParseRecord();
  ASSERT_TRUE(record2.holds<Mbp10Msg>());
  const auto& mbp2 = record2.get<Mbp10Msg>();
  EXPECT_EQ(mbp2.hd.publisher_id, 1);
  EXPECT_EQ(mbp2.hd.product_id, 5482);
  EXPECT_EQ(mbp2.hd.ts_event.time_since_epoch().count(), 1609160400000435673);
  EXPECT_EQ(mbp2.price, 3720000000000);
  EXPECT_EQ(mbp2.size, 1);
  EXPECT_EQ(mbp2.action, 'C');
  EXPECT_EQ(mbp2.side, 'B');
  EXPECT_EQ(mbp2.flags, -128);
  EXPECT_EQ(mbp2.depth, 1);
  EXPECT_EQ(mbp2.ts_recv.time_since_epoch().count(), 1609160400000750544);
  EXPECT_EQ(mbp2.ts_in_delta.count(), 20625);
  EXPECT_EQ(mbp2.sequence, 1170356);
  EXPECT_EQ(mbp2.booklevel[0].bid_px, 3720250000000);
  EXPECT_EQ(mbp2.booklevel[0].ask_px, 3720500000000);
  EXPECT_EQ(mbp2.booklevel[0].bid_sz, 24);
  EXPECT_EQ(mbp2.booklevel[0].ask_sz, 10);
  EXPECT_EQ(mbp2.booklevel[0].bid_ct, 15);
  EXPECT_EQ(mbp2.booklevel[0].ask_ct, 8);
  EXPECT_EQ(mbp2.booklevel[1].bid_px, 3720000000000);
  EXPECT_EQ(mbp2.booklevel[1].ask_px, 3720750000000);
  EXPECT_EQ(mbp2.booklevel[1].bid_sz, 30);
  EXPECT_EQ(mbp2.booklevel[1].ask_sz, 34);
  EXPECT_EQ(mbp2.booklevel[1].bid_ct, 17);
  EXPECT_EQ(mbp2.booklevel[1].ask_ct, 24);
  EXPECT_EQ(mbp2.booklevel[2].bid_px, 3719750000000);
  EXPECT_EQ(mbp2.booklevel[2].ask_px, 3721000000000);
  EXPECT_EQ(mbp2.booklevel[2].bid_sz, 32);
  EXPECT_EQ(mbp2.booklevel[2].ask_sz, 39);
  EXPECT_EQ(mbp2.booklevel[2].bid_ct, 23);
  EXPECT_EQ(mbp2.booklevel[2].ask_ct, 25);
}

TEST_F(DbzParserTests, TestParseTbbo) {
  write_thread_ = std::thread([this] {
    this->ReadFromFile(TEST_BUILD_DIR "/data/test_data.tbbo.dbz");
  });

  const Metadata metadata = target_.ParseMetadata();
  EXPECT_EQ(metadata.version, 1);
  EXPECT_EQ(metadata.dataset, "GLBX.MDP3");
  EXPECT_EQ(metadata.schema, Schema::Tbbo);
  EXPECT_EQ(metadata.start.time_since_epoch().count(), 1609160400000000000);
  EXPECT_EQ(metadata.end.time_since_epoch().count(), 1609200000000000000);
  EXPECT_EQ(metadata.limit, 2);
  EXPECT_EQ(metadata.record_count, 2);
  EXPECT_EQ(metadata.stype_in, SType::Native);
  EXPECT_EQ(metadata.stype_out, SType::ProductId);
  EXPECT_EQ(metadata.symbols, std::vector<std::string>{"ESH1"});
  EXPECT_TRUE(metadata.partial.empty());
  EXPECT_TRUE(metadata.not_found.empty());
  AssertMappings(metadata.mappings);

  const auto record1 = target_.ParseRecord();
  ASSERT_TRUE(record1.holds<TbboMsg>());
  const auto& tbbo1 = record1.get<TbboMsg>();
  EXPECT_EQ(tbbo1.hd.publisher_id, 1);
  EXPECT_EQ(tbbo1.hd.product_id, 5482);
  EXPECT_EQ(tbbo1.hd.ts_event.time_since_epoch().count(), 1609160400098821953);
  EXPECT_EQ(tbbo1.price, 3720250000000);
  EXPECT_EQ(tbbo1.size, 5);
  EXPECT_EQ(tbbo1.action, 'T');
  EXPECT_EQ(tbbo1.side, 'A');
  EXPECT_EQ(tbbo1.flags, -127);
  EXPECT_EQ(tbbo1.depth, 0);
  EXPECT_EQ(tbbo1.ts_recv.time_since_epoch().count(), 1609160400099150057);
  EXPECT_EQ(tbbo1.ts_in_delta.count(), 19251);
  EXPECT_EQ(tbbo1.sequence, 1170380);
  EXPECT_EQ(tbbo1.booklevel[0].bid_px, 3720250000000);
  EXPECT_EQ(tbbo1.booklevel[0].ask_px, 3720500000000);
  EXPECT_EQ(tbbo1.booklevel[0].bid_sz, 26);
  EXPECT_EQ(tbbo1.booklevel[0].ask_sz, 7);
  EXPECT_EQ(tbbo1.booklevel[0].bid_ct, 16);
  EXPECT_EQ(tbbo1.booklevel[0].ask_ct, 6);

  const auto record2 = target_.ParseRecord();
  ASSERT_TRUE(record2.holds<TbboMsg>());
  const auto& tbbo2 = record2.get<TbboMsg>();
  EXPECT_EQ(tbbo2.hd.publisher_id, 1);
  EXPECT_EQ(tbbo2.hd.product_id, 5482);
  EXPECT_EQ(tbbo2.hd.ts_event.time_since_epoch().count(), 1609160400107665963);
  EXPECT_EQ(tbbo2.price, 3720250000000);
  EXPECT_EQ(tbbo2.size, 21);
  EXPECT_EQ(tbbo2.action, 'T');
  EXPECT_EQ(tbbo2.side, 'A');
  EXPECT_EQ(tbbo2.flags, -127);
  EXPECT_EQ(tbbo2.depth, 0);
  EXPECT_EQ(tbbo2.ts_recv.time_since_epoch().count(), 1609160400108142648);
  EXPECT_EQ(tbbo2.ts_in_delta.count(), 20728);
  EXPECT_EQ(tbbo2.sequence, 1170414);
  EXPECT_EQ(tbbo2.booklevel[0].bid_px, 3720250000000);
  EXPECT_EQ(tbbo2.booklevel[0].ask_px, 3720500000000);
  EXPECT_EQ(tbbo2.booklevel[0].bid_sz, 21);
  EXPECT_EQ(tbbo2.booklevel[0].ask_sz, 22);
  EXPECT_EQ(tbbo2.booklevel[0].bid_ct, 13);
  EXPECT_EQ(tbbo2.booklevel[0].ask_ct, 15);
}

TEST_F(DbzParserTests, TestParseTrades) {
  write_thread_ = std::thread([this] {
    this->ReadFromFile(TEST_BUILD_DIR "/data/test_data.trades.dbz");
  });

  const Metadata metadata = target_.ParseMetadata();
  EXPECT_EQ(metadata.version, 1);
  EXPECT_EQ(metadata.dataset, "GLBX.MDP3");
  EXPECT_EQ(metadata.schema, Schema::Trades);
  EXPECT_EQ(metadata.start.time_since_epoch().count(), 1609160400000000000);
  EXPECT_EQ(metadata.end.time_since_epoch().count(), 1609200000000000000);
  EXPECT_EQ(metadata.limit, 2);
  EXPECT_EQ(metadata.record_count, 2);
  EXPECT_EQ(metadata.stype_in, SType::Native);
  EXPECT_EQ(metadata.stype_out, SType::ProductId);
  EXPECT_EQ(metadata.symbols, std::vector<std::string>{"ESH1"});
  EXPECT_TRUE(metadata.partial.empty());
  EXPECT_TRUE(metadata.not_found.empty());
  AssertMappings(metadata.mappings);

  const auto record1 = target_.ParseRecord();
  ASSERT_TRUE(record1.holds<TradeMsg>());
  const auto& trade1 = record1.get<TradeMsg>();
  EXPECT_EQ(trade1.hd.publisher_id, 1);
  EXPECT_EQ(trade1.hd.product_id, 5482);
  EXPECT_EQ(trade1.hd.ts_event.time_since_epoch().count(), 1609160400098821953);
  EXPECT_EQ(trade1.price, 3720250000000);
  EXPECT_EQ(trade1.size, 5);
  EXPECT_EQ(trade1.action, 'T');
  EXPECT_EQ(trade1.side, 'A');
  EXPECT_EQ(trade1.flags, -127);
  EXPECT_EQ(trade1.depth, 0);
  EXPECT_EQ(trade1.ts_recv.time_since_epoch().count(), 1609160400099150057);
  EXPECT_EQ(trade1.ts_in_delta.count(), 19251);
  EXPECT_EQ(trade1.sequence, 1170380);

  const auto record2 = target_.ParseRecord();
  ASSERT_TRUE(record2.holds<TradeMsg>());
  const auto& trade2 = record2.get<TradeMsg>();
  EXPECT_EQ(trade2.hd.publisher_id, 1);
  EXPECT_EQ(trade2.hd.product_id, 5482);
  EXPECT_EQ(trade2.hd.ts_event.time_since_epoch().count(), 1609160400107665963);
  EXPECT_EQ(trade2.price, 3720250000000);
  EXPECT_EQ(trade2.size, 21);
  EXPECT_EQ(trade2.action, 'T');
  EXPECT_EQ(trade2.side, 'A');
  EXPECT_EQ(trade2.flags, -127);
  EXPECT_EQ(trade2.depth, 0);
  EXPECT_EQ(trade2.ts_recv.time_since_epoch().count(), 1609160400108142648);
  EXPECT_EQ(trade2.ts_in_delta.count(), 20728);
  EXPECT_EQ(trade2.sequence, 1170414);
}

TEST_F(DbzParserTests, TestParseOhlcv1H) {
  write_thread_ = std::thread([this] {
    this->ReadFromFile(TEST_BUILD_DIR "/data/test_data.ohlcv-1h.dbz");
  });

  const Metadata metadata = target_.ParseMetadata();
  EXPECT_EQ(metadata.version, 1);
  EXPECT_EQ(metadata.dataset, "GLBX.MDP3");
  EXPECT_EQ(metadata.schema, Schema::Ohlcv1H);
  EXPECT_EQ(metadata.start.time_since_epoch().count(), 1609160400000000000);
  EXPECT_EQ(metadata.end.time_since_epoch().count(), 1609200000000000000);
  EXPECT_EQ(metadata.limit, 2);
  EXPECT_EQ(metadata.record_count, 2);
  EXPECT_EQ(metadata.stype_in, SType::Native);
  EXPECT_EQ(metadata.stype_out, SType::ProductId);
  EXPECT_EQ(metadata.symbols, std::vector<std::string>{"ESH1"});
  EXPECT_TRUE(metadata.partial.empty());
  EXPECT_TRUE(metadata.not_found.empty());
  AssertMappings(metadata.mappings);

  const auto record1 = target_.ParseRecord();
  ASSERT_TRUE(record1.holds<OhlcvMsg>());
  const auto& ohlcv1 = record1.get<OhlcvMsg>();
  EXPECT_EQ(ohlcv1.hd.publisher_id, 1);
  EXPECT_EQ(ohlcv1.hd.product_id, 5482);
  EXPECT_EQ(ohlcv1.hd.ts_event.time_since_epoch().count(), 1609160400000000000);
  EXPECT_EQ(ohlcv1.open, 372025000000000);
  EXPECT_EQ(ohlcv1.high, 372350000000000);
  EXPECT_EQ(ohlcv1.low, 372025000000000);
  EXPECT_EQ(ohlcv1.close, 372225000000000);
  EXPECT_EQ(ohlcv1.volume, 9385);

  const auto record2 = target_.ParseRecord();
  ASSERT_TRUE(record2.holds<OhlcvMsg>());
  const auto& ohlcv2 = record2.get<OhlcvMsg>();
  EXPECT_EQ(ohlcv2.hd.publisher_id, 1);
  EXPECT_EQ(ohlcv2.hd.product_id, 5482);
  EXPECT_EQ(ohlcv2.hd.ts_event.time_since_epoch().count(), 1609164000000000000);
  EXPECT_EQ(ohlcv1.open, 372225000000000);
  EXPECT_EQ(ohlcv1.high, 372450000000000);
  EXPECT_EQ(ohlcv1.low, 371600000000000);
  EXPECT_EQ(ohlcv1.close, 371950000000000);
  EXPECT_EQ(ohlcv1.volume, 112698);
}

TEST_F(DbzParserTests, TestParseOhlcv1M) {
  write_thread_ = std::thread([this] {
    this->ReadFromFile(TEST_BUILD_DIR "/data/test_data.ohlcv-1m.dbz");
  });

  const Metadata metadata = target_.ParseMetadata();
  EXPECT_EQ(metadata.version, 1);
  EXPECT_EQ(metadata.dataset, "GLBX.MDP3");
  EXPECT_EQ(metadata.schema, Schema::Ohlcv1M);
  EXPECT_EQ(metadata.start.time_since_epoch().count(), 1609160400000000000);
  EXPECT_EQ(metadata.end.time_since_epoch().count(), 1609200000000000000);
  EXPECT_EQ(metadata.limit, 2);
  EXPECT_EQ(metadata.record_count, 2);
  EXPECT_EQ(metadata.stype_in, SType::Native);
  EXPECT_EQ(metadata.stype_out, SType::ProductId);
  EXPECT_EQ(metadata.symbols, std::vector<std::string>{"ESH1"});
  EXPECT_TRUE(metadata.partial.empty());
  EXPECT_TRUE(metadata.not_found.empty());
  AssertMappings(metadata.mappings);

  const auto record1 = target_.ParseRecord();
  ASSERT_TRUE(record1.holds<OhlcvMsg>());
  const auto& ohlcv1 = record1.get<OhlcvMsg>();
  EXPECT_EQ(ohlcv1.hd.publisher_id, 1);
  EXPECT_EQ(ohlcv1.hd.product_id, 5482);
  EXPECT_EQ(ohlcv1.hd.ts_event.time_since_epoch().count(), 1609160400000000000);
  EXPECT_EQ(ohlcv1.open, 372025000000000);
  EXPECT_EQ(ohlcv1.high, 372150000000000);
  EXPECT_EQ(ohlcv1.low, 372025000000000);
  EXPECT_EQ(ohlcv1.close, 372100000000000);
  EXPECT_EQ(ohlcv1.volume, 353);

  const auto record2 = target_.ParseRecord();
  ASSERT_TRUE(record2.holds<OhlcvMsg>());
  const auto& ohlcv2 = record2.get<OhlcvMsg>();
  EXPECT_EQ(ohlcv2.hd.publisher_id, 1);
  EXPECT_EQ(ohlcv2.hd.product_id, 5482);
  EXPECT_EQ(ohlcv2.hd.ts_event.time_since_epoch().count(), 1609160460000000000);
  EXPECT_EQ(ohlcv1.open, 372100000000000);
  EXPECT_EQ(ohlcv1.high, 372150000000000);
  EXPECT_EQ(ohlcv1.low, 372100000000000);
  EXPECT_EQ(ohlcv1.close, 372150000000000);
  EXPECT_EQ(ohlcv1.volume, 152);
}

TEST_F(DbzParserTests, TestParseOhlcv1S) {
  write_thread_ = std::thread([this] {
    this->ReadFromFile(TEST_BUILD_DIR "/data/test_data.ohlcv-1s.dbz");
  });

  const Metadata metadata = target_.ParseMetadata();
  EXPECT_EQ(metadata.version, 1);
  EXPECT_EQ(metadata.dataset, "GLBX.MDP3");
  EXPECT_EQ(metadata.schema, Schema::Ohlcv1S);
  EXPECT_EQ(metadata.start.time_since_epoch().count(), 1609160400000000000);
  EXPECT_EQ(metadata.end.time_since_epoch().count(), 1609200000000000000);
  EXPECT_EQ(metadata.limit, 2);
  EXPECT_EQ(metadata.record_count, 2);
  EXPECT_EQ(metadata.stype_in, SType::Native);
  EXPECT_EQ(metadata.stype_out, SType::ProductId);
  EXPECT_EQ(metadata.symbols, std::vector<std::string>{"ESH1"});
  EXPECT_TRUE(metadata.partial.empty());
  EXPECT_TRUE(metadata.not_found.empty());
  AssertMappings(metadata.mappings);

  const auto record1 = target_.ParseRecord();
  ASSERT_TRUE(record1.holds<OhlcvMsg>());
  const auto& ohlcv1 = record1.get<OhlcvMsg>();
  EXPECT_EQ(ohlcv1.hd.publisher_id, 1);
  EXPECT_EQ(ohlcv1.hd.product_id, 5482);
  EXPECT_EQ(ohlcv1.hd.ts_event.time_since_epoch().count(), 1609160400000000000);
  EXPECT_EQ(ohlcv1.open, 372025000000000);
  EXPECT_EQ(ohlcv1.high, 372050000000000);
  EXPECT_EQ(ohlcv1.low, 372025000000000);
  EXPECT_EQ(ohlcv1.close, 372050000000000);
  EXPECT_EQ(ohlcv1.volume, 57);

  const auto record2 = target_.ParseRecord();
  ASSERT_TRUE(record2.holds<OhlcvMsg>());
  const auto& ohlcv2 = record2.get<OhlcvMsg>();
  EXPECT_EQ(ohlcv2.hd.publisher_id, 1);
  EXPECT_EQ(ohlcv2.hd.product_id, 5482);
  EXPECT_EQ(ohlcv2.hd.ts_event.time_since_epoch().count(), 1609160401000000000);
  EXPECT_EQ(ohlcv1.open, 372050000000000);
  EXPECT_EQ(ohlcv1.high, 372050000000000);
  EXPECT_EQ(ohlcv1.low, 372050000000000);
  EXPECT_EQ(ohlcv1.close, 372050000000000);
  EXPECT_EQ(ohlcv1.volume, 13);
}
}  // namespace test
}  // namespace databento
