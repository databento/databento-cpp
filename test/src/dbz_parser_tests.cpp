#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <fstream>  // ifstream
#include <ios>      // streamsize, ios::binary, ios::ate
#include <memory>
#include <thread>

#include "databento/constants.hpp"
#include "databento/dbz.hpp"
#include "databento/dbz_parser.hpp"
#include "databento/detail/file_stream.hpp"
#include "databento/detail/shared_channel.hpp"
#include "databento/enums.hpp"
#include "databento/record.hpp"

namespace databento {
namespace test {
class DbzParserTests : public testing::Test {
 protected:
  detail::SharedChannel channel_;
  DbzChannelParser channel_target_{channel_};
  std::unique_ptr<DbzFileParser> file_target_;
  std::thread write_thread_;

  void TearDown() override {
    if (write_thread_.joinable()) {
      write_thread_.join();
    }
  }

  void ReadFromFile(const std::string& file_path) {
    // Channel setup
    write_thread_ = std::thread{[this, file_path] {
      // ifstream with std::uint8_t compiles but doesn't work
      std::ifstream input_file{file_path, std::ios::binary | std::ios::ate};
      ASSERT_TRUE(input_file.good());
      const auto size = static_cast<std::size_t>(input_file.tellg());
      input_file.seekg(0, std::ios::beg);
      std::vector<char> buffer(size);
      input_file.read(buffer.data(), static_cast<std::streamsize>(size));
      channel_.Write(reinterpret_cast<const std::uint8_t*>(buffer.data()),
                     size);
      channel_.Finish();
    }};
    // File setup
    file_target_.reset(new DbzFileParser{detail::FileStream{file_path}});
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
  ReadFromFile(TEST_BUILD_DIR "/data/test_data.mbo.dbz");

  const Metadata ch_metadata = channel_target_.ParseMetadata();
  const Metadata f_metadata = file_target_->ParseMetadata();
  EXPECT_EQ(ch_metadata, f_metadata);
  EXPECT_EQ(ch_metadata.version, 1);
  EXPECT_EQ(ch_metadata.dataset, dataset::kGlbxMdp3);
  EXPECT_EQ(ch_metadata.schema, Schema::Mbo);
  EXPECT_EQ(ch_metadata.start.time_since_epoch().count(), 1609160400000000000);
  EXPECT_EQ(ch_metadata.end.time_since_epoch().count(), 1609200000000000000);
  EXPECT_EQ(ch_metadata.limit, 2);
  EXPECT_EQ(ch_metadata.record_count, 2);
  EXPECT_EQ(ch_metadata.stype_in, SType::Native);
  EXPECT_EQ(ch_metadata.stype_out, SType::ProductId);
  EXPECT_EQ(ch_metadata.symbols, std::vector<std::string>{"ESH1"});
  EXPECT_TRUE(ch_metadata.partial.empty());
  EXPECT_TRUE(ch_metadata.not_found.empty());

  const auto ch_record1 = channel_target_.ParseRecord();
  const auto f_record1 = file_target_->ParseRecord();
  ASSERT_TRUE(ch_record1.Holds<MboMsg>());
  ASSERT_TRUE(f_record1.Holds<MboMsg>());
  const auto& ch_mbo1 = ch_record1.Get<MboMsg>();
  const auto& f_mbo1 = f_record1.Get<MboMsg>();
  EXPECT_EQ(ch_mbo1, f_mbo1);
  EXPECT_EQ(ch_mbo1.hd.publisher_id, 1);
  EXPECT_EQ(ch_mbo1.hd.product_id, 5482);
  EXPECT_EQ(ch_mbo1.hd.ts_event.time_since_epoch().count(),
            1609160400000429831);
  EXPECT_EQ(ch_mbo1.order_id, 647784973705);
  EXPECT_EQ(ch_mbo1.price, 3722750000000);
  EXPECT_EQ(ch_mbo1.size, 1);
  EXPECT_EQ(ch_mbo1.flags, 128);
  EXPECT_EQ(ch_mbo1.channel_id, 0);
  EXPECT_EQ(ch_mbo1.action, 'C');
  EXPECT_EQ(ch_mbo1.side, 'A');
  EXPECT_EQ(ch_mbo1.ts_recv.time_since_epoch().count(), 1609160400000704060);
  EXPECT_EQ(ch_mbo1.ts_in_delta.count(), 22993);
  EXPECT_EQ(ch_mbo1.sequence, 1170352);

  const auto ch_record2 = channel_target_.ParseRecord();
  ASSERT_TRUE(ch_record2.Holds<MboMsg>());
  const auto f_record2 = file_target_->ParseRecord();
  ASSERT_TRUE(f_record2.Holds<MboMsg>());
  const auto& ch_mbo2 = ch_record2.Get<MboMsg>();
  const auto& f_mbo2 = f_record2.Get<MboMsg>();
  EXPECT_EQ(ch_mbo2, f_mbo2);
  EXPECT_EQ(ch_mbo2.hd.publisher_id, 1);
  EXPECT_EQ(ch_mbo2.hd.product_id, 5482);
  EXPECT_EQ(ch_mbo2.hd.ts_event.time_since_epoch().count(),
            1609160400000431665);
  EXPECT_EQ(ch_mbo2.order_id, 647784973631);
  EXPECT_EQ(ch_mbo2.price, 3723000000000);
  EXPECT_EQ(ch_mbo2.size, 1);
  EXPECT_EQ(ch_mbo2.flags, 128);
  EXPECT_EQ(ch_mbo2.channel_id, 0);
  EXPECT_EQ(ch_mbo2.action, 'C');
  EXPECT_EQ(ch_mbo2.side, 'A');
  EXPECT_EQ(ch_mbo2.ts_recv.time_since_epoch().count(), 1609160400000711344);
  EXPECT_EQ(ch_mbo2.ts_in_delta.count(), 19621);
  EXPECT_EQ(ch_mbo2.sequence, 1170353);
}

TEST_F(DbzParserTests, TestParseMbp1) {
  this->ReadFromFile(TEST_BUILD_DIR "/data/test_data.mbp-1.dbz");

  const Metadata ch_metadata = channel_target_.ParseMetadata();
  const Metadata f_metadata = file_target_->ParseMetadata();
  EXPECT_EQ(ch_metadata, f_metadata);
  EXPECT_EQ(ch_metadata.version, 1);
  EXPECT_EQ(ch_metadata.dataset, dataset::kGlbxMdp3);
  EXPECT_EQ(ch_metadata.schema, Schema::Mbp1);
  EXPECT_EQ(ch_metadata.start.time_since_epoch().count(), 1609160400000000000);
  EXPECT_EQ(ch_metadata.end.time_since_epoch().count(), 1609200000000000000);
  EXPECT_EQ(ch_metadata.limit, 2);
  EXPECT_EQ(ch_metadata.record_count, 2);
  EXPECT_EQ(ch_metadata.stype_in, SType::Native);
  EXPECT_EQ(ch_metadata.stype_out, SType::ProductId);
  EXPECT_EQ(ch_metadata.symbols, std::vector<std::string>{"ESH1"});
  EXPECT_TRUE(ch_metadata.partial.empty());
  EXPECT_TRUE(ch_metadata.not_found.empty());
  AssertMappings(ch_metadata.mappings);

  const auto ch_record1 = channel_target_.ParseRecord();
  const auto f_record1 = file_target_->ParseRecord();
  ASSERT_TRUE(ch_record1.Holds<Mbp1Msg>());
  ASSERT_TRUE(f_record1.Holds<Mbp1Msg>());
  const auto& ch_mbp1 = ch_record1.Get<Mbp1Msg>();
  const auto& f_mbp1 = f_record1.Get<Mbp1Msg>();
  EXPECT_EQ(ch_mbp1, f_mbp1);
  EXPECT_EQ(ch_mbp1.hd.publisher_id, 1);
  EXPECT_EQ(ch_mbp1.hd.product_id, 5482);
  EXPECT_EQ(ch_mbp1.hd.ts_event.time_since_epoch().count(),
            1609160400006001487);
  EXPECT_EQ(ch_mbp1.price, 3720500000000);
  EXPECT_EQ(ch_mbp1.size, 1);
  EXPECT_EQ(ch_mbp1.action, 'A');
  EXPECT_EQ(ch_mbp1.side, 'A');
  EXPECT_EQ(ch_mbp1.flags, 128);
  EXPECT_EQ(ch_mbp1.depth, 0);
  EXPECT_EQ(ch_mbp1.ts_recv.time_since_epoch().count(), 1609160400006136329);
  EXPECT_EQ(ch_mbp1.ts_in_delta.count(), 17214);
  EXPECT_EQ(ch_mbp1.sequence, 1170362);
  EXPECT_EQ(ch_mbp1.booklevel[0].bid_px, 3720250000000);
  EXPECT_EQ(ch_mbp1.booklevel[0].ask_px, 3720500000000);
  EXPECT_EQ(ch_mbp1.booklevel[0].bid_sz, 24);
  EXPECT_EQ(ch_mbp1.booklevel[0].ask_sz, 11);
  EXPECT_EQ(ch_mbp1.booklevel[0].bid_ct, 15);
  EXPECT_EQ(ch_mbp1.booklevel[0].ask_ct, 9);

  const auto ch_record2 = channel_target_.ParseRecord();
  const auto f_record2 = file_target_->ParseRecord();
  ASSERT_TRUE(ch_record2.Holds<Mbp1Msg>());
  ASSERT_TRUE(f_record2.Holds<Mbp1Msg>());
  const auto& ch_mbp2 = ch_record2.Get<Mbp1Msg>();
  const auto& f_mbp2 = f_record2.Get<Mbp1Msg>();
  EXPECT_EQ(ch_mbp2, f_mbp2);
  EXPECT_EQ(ch_mbp2.hd.publisher_id, 1);
  EXPECT_EQ(ch_mbp2.hd.product_id, 5482);
  EXPECT_EQ(ch_mbp2.hd.ts_event.time_since_epoch().count(),
            1609160400006146661);
  EXPECT_EQ(ch_mbp2.price, 3720500000000);
  EXPECT_EQ(ch_mbp2.size, 1);
  EXPECT_EQ(ch_mbp2.action, 'A');
  EXPECT_EQ(ch_mbp2.side, 'A');
  EXPECT_EQ(ch_mbp2.flags, 128);
  EXPECT_EQ(ch_mbp2.depth, 0);
  EXPECT_EQ(ch_mbp2.ts_recv.time_since_epoch().count(), 1609160400006246513);
  EXPECT_EQ(ch_mbp2.ts_in_delta.count(), 18858);
  EXPECT_EQ(ch_mbp2.sequence, 1170364);
  EXPECT_EQ(ch_mbp2.booklevel[0].bid_px, 3720250000000);
  EXPECT_EQ(ch_mbp2.booklevel[0].ask_px, 3720500000000);
  EXPECT_EQ(ch_mbp2.booklevel[0].bid_sz, 24);
  EXPECT_EQ(ch_mbp2.booklevel[0].ask_sz, 12);
  EXPECT_EQ(ch_mbp2.booklevel[0].bid_ct, 15);
  EXPECT_EQ(ch_mbp2.booklevel[0].ask_ct, 10);
}

TEST_F(DbzParserTests, TestParseMbp10) {
  this->ReadFromFile(TEST_BUILD_DIR "/data/test_data.mbp-10.dbz");

  const Metadata ch_metadata = channel_target_.ParseMetadata();
  const Metadata f_metadata = file_target_->ParseMetadata();
  EXPECT_EQ(ch_metadata, f_metadata);
  EXPECT_EQ(ch_metadata.version, 1);
  EXPECT_EQ(ch_metadata.dataset, dataset::kGlbxMdp3);
  EXPECT_EQ(ch_metadata.schema, Schema::Mbp10);
  EXPECT_EQ(ch_metadata.start.time_since_epoch().count(), 1609160400000000000);
  EXPECT_EQ(ch_metadata.end.time_since_epoch().count(), 1609200000000000000);
  EXPECT_EQ(ch_metadata.limit, 2);
  EXPECT_EQ(ch_metadata.record_count, 2);
  EXPECT_EQ(ch_metadata.stype_in, SType::Native);
  EXPECT_EQ(ch_metadata.stype_out, SType::ProductId);
  EXPECT_EQ(ch_metadata.symbols, std::vector<std::string>{"ESH1"});
  EXPECT_TRUE(ch_metadata.partial.empty());
  EXPECT_TRUE(ch_metadata.not_found.empty());
  AssertMappings(ch_metadata.mappings);

  const auto ch_record1 = channel_target_.ParseRecord();
  const auto f_record1 = file_target_->ParseRecord();
  ASSERT_TRUE(ch_record1.Holds<Mbp10Msg>());
  ASSERT_TRUE(f_record1.Holds<Mbp10Msg>());
  const auto& ch_mbp1 = ch_record1.Get<Mbp10Msg>();
  const auto& f_mbp1 = f_record1.Get<Mbp10Msg>();
  EXPECT_EQ(ch_mbp1, f_mbp1);
  EXPECT_EQ(ch_mbp1.hd.publisher_id, 1);
  EXPECT_EQ(ch_mbp1.hd.product_id, 5482);
  EXPECT_EQ(ch_mbp1.hd.ts_event.time_since_epoch().count(),
            1609160400000429831);
  EXPECT_EQ(ch_mbp1.price, 3722750000000);
  EXPECT_EQ(ch_mbp1.size, 1);
  EXPECT_EQ(ch_mbp1.action, 'C');
  EXPECT_EQ(ch_mbp1.side, 'A');
  EXPECT_EQ(ch_mbp1.flags, 128);
  EXPECT_EQ(ch_mbp1.depth, 9);
  EXPECT_EQ(ch_mbp1.ts_recv.time_since_epoch().count(), 1609160400000704060);
  EXPECT_EQ(ch_mbp1.ts_in_delta.count(), 22993);
  EXPECT_EQ(ch_mbp1.sequence, 1170352);
  EXPECT_EQ(ch_mbp1.booklevel[0].bid_px, 3720250000000);
  EXPECT_EQ(ch_mbp1.booklevel[0].ask_px, 3720500000000);
  EXPECT_EQ(ch_mbp1.booklevel[0].bid_sz, 24);
  EXPECT_EQ(ch_mbp1.booklevel[0].ask_sz, 10);
  EXPECT_EQ(ch_mbp1.booklevel[0].bid_ct, 15);
  EXPECT_EQ(ch_mbp1.booklevel[0].ask_ct, 8);
  EXPECT_EQ(ch_mbp1.booklevel[1].bid_px, 3720000000000);
  EXPECT_EQ(ch_mbp1.booklevel[1].ask_px, 3720750000000);
  EXPECT_EQ(ch_mbp1.booklevel[1].bid_sz, 31);
  EXPECT_EQ(ch_mbp1.booklevel[1].ask_sz, 34);
  EXPECT_EQ(ch_mbp1.booklevel[1].bid_ct, 18);
  EXPECT_EQ(ch_mbp1.booklevel[1].ask_ct, 24);
  EXPECT_EQ(ch_mbp1.booklevel[2].bid_px, 3719750000000);
  EXPECT_EQ(ch_mbp1.booklevel[2].ask_px, 3721000000000);
  EXPECT_EQ(ch_mbp1.booklevel[2].bid_sz, 32);
  EXPECT_EQ(ch_mbp1.booklevel[2].ask_sz, 39);
  EXPECT_EQ(ch_mbp1.booklevel[2].bid_ct, 23);
  EXPECT_EQ(ch_mbp1.booklevel[2].ask_ct, 25);

  const auto ch_record2 = channel_target_.ParseRecord();
  const auto f_record2 = file_target_->ParseRecord();
  ASSERT_TRUE(ch_record2.Holds<Mbp10Msg>());
  ASSERT_TRUE(f_record2.Holds<Mbp10Msg>());
  const auto& ch_mbp2 = ch_record2.Get<Mbp10Msg>();
  const auto& f_mbp2 = f_record2.Get<Mbp10Msg>();
  EXPECT_EQ(ch_mbp2, f_mbp2);
  EXPECT_EQ(ch_mbp2.hd.publisher_id, 1);
  EXPECT_EQ(ch_mbp2.hd.product_id, 5482);
  EXPECT_EQ(ch_mbp2.hd.ts_event.time_since_epoch().count(),
            1609160400000435673);
  EXPECT_EQ(ch_mbp2.price, 3720000000000);
  EXPECT_EQ(ch_mbp2.size, 1);
  EXPECT_EQ(ch_mbp2.action, 'C');
  EXPECT_EQ(ch_mbp2.side, 'B');
  EXPECT_EQ(ch_mbp2.flags, 128);
  EXPECT_EQ(ch_mbp2.depth, 1);
  EXPECT_EQ(ch_mbp2.ts_recv.time_since_epoch().count(), 1609160400000750544);
  EXPECT_EQ(ch_mbp2.ts_in_delta.count(), 20625);
  EXPECT_EQ(ch_mbp2.sequence, 1170356);
  EXPECT_EQ(ch_mbp2.booklevel[0].bid_px, 3720250000000);
  EXPECT_EQ(ch_mbp2.booklevel[0].ask_px, 3720500000000);
  EXPECT_EQ(ch_mbp2.booklevel[0].bid_sz, 24);
  EXPECT_EQ(ch_mbp2.booklevel[0].ask_sz, 10);
  EXPECT_EQ(ch_mbp2.booklevel[0].bid_ct, 15);
  EXPECT_EQ(ch_mbp2.booklevel[0].ask_ct, 8);
  EXPECT_EQ(ch_mbp2.booklevel[1].bid_px, 3720000000000);
  EXPECT_EQ(ch_mbp2.booklevel[1].ask_px, 3720750000000);
  EXPECT_EQ(ch_mbp2.booklevel[1].bid_sz, 30);
  EXPECT_EQ(ch_mbp2.booklevel[1].ask_sz, 34);
  EXPECT_EQ(ch_mbp2.booklevel[1].bid_ct, 17);
  EXPECT_EQ(ch_mbp2.booklevel[1].ask_ct, 24);
  EXPECT_EQ(ch_mbp2.booklevel[2].bid_px, 3719750000000);
  EXPECT_EQ(ch_mbp2.booklevel[2].ask_px, 3721000000000);
  EXPECT_EQ(ch_mbp2.booklevel[2].bid_sz, 32);
  EXPECT_EQ(ch_mbp2.booklevel[2].ask_sz, 39);
  EXPECT_EQ(ch_mbp2.booklevel[2].bid_ct, 23);
  EXPECT_EQ(ch_mbp2.booklevel[2].ask_ct, 25);
}

TEST_F(DbzParserTests, TestParseTbbo) {
  this->ReadFromFile(TEST_BUILD_DIR "/data/test_data.tbbo.dbz");

  const Metadata ch_metadata = channel_target_.ParseMetadata();
  const Metadata f_metadata = file_target_->ParseMetadata();
  EXPECT_EQ(ch_metadata, f_metadata);
  EXPECT_EQ(ch_metadata.version, 1);
  EXPECT_EQ(ch_metadata.dataset, dataset::kGlbxMdp3);
  EXPECT_EQ(ch_metadata.schema, Schema::Tbbo);
  EXPECT_EQ(ch_metadata.start.time_since_epoch().count(), 1609160400000000000);
  EXPECT_EQ(ch_metadata.end.time_since_epoch().count(), 1609200000000000000);
  EXPECT_EQ(ch_metadata.limit, 2);
  EXPECT_EQ(ch_metadata.record_count, 2);
  EXPECT_EQ(ch_metadata.stype_in, SType::Native);
  EXPECT_EQ(ch_metadata.stype_out, SType::ProductId);
  EXPECT_EQ(ch_metadata.symbols, std::vector<std::string>{"ESH1"});
  EXPECT_TRUE(ch_metadata.partial.empty());
  EXPECT_TRUE(ch_metadata.not_found.empty());
  AssertMappings(ch_metadata.mappings);

  const auto ch_record1 = channel_target_.ParseRecord();
  const auto f_record1 = file_target_->ParseRecord();
  ASSERT_TRUE(ch_record1.Holds<TbboMsg>());
  ASSERT_TRUE(f_record1.Holds<TbboMsg>());
  const auto& ch_tbbo1 = ch_record1.Get<TbboMsg>();
  const auto& f_tbbo1 = f_record1.Get<TbboMsg>();
  EXPECT_EQ(ch_tbbo1, f_tbbo1);
  EXPECT_EQ(ch_tbbo1.hd.publisher_id, 1);
  EXPECT_EQ(ch_tbbo1.hd.product_id, 5482);
  EXPECT_EQ(ch_tbbo1.hd.ts_event.time_since_epoch().count(),
            1609160400098821953);
  EXPECT_EQ(ch_tbbo1.price, 3720250000000);
  EXPECT_EQ(ch_tbbo1.size, 5);
  EXPECT_EQ(ch_tbbo1.action, 'T');
  EXPECT_EQ(ch_tbbo1.side, 'A');
  EXPECT_EQ(ch_tbbo1.flags, 129);
  EXPECT_EQ(ch_tbbo1.depth, 0);
  EXPECT_EQ(ch_tbbo1.ts_recv.time_since_epoch().count(), 1609160400099150057);
  EXPECT_EQ(ch_tbbo1.ts_in_delta.count(), 19251);
  EXPECT_EQ(ch_tbbo1.sequence, 1170380);
  EXPECT_EQ(ch_tbbo1.booklevel[0].bid_px, 3720250000000);
  EXPECT_EQ(ch_tbbo1.booklevel[0].ask_px, 3720500000000);
  EXPECT_EQ(ch_tbbo1.booklevel[0].bid_sz, 26);
  EXPECT_EQ(ch_tbbo1.booklevel[0].ask_sz, 7);
  EXPECT_EQ(ch_tbbo1.booklevel[0].bid_ct, 16);
  EXPECT_EQ(ch_tbbo1.booklevel[0].ask_ct, 6);

  const auto ch_record2 = channel_target_.ParseRecord();
  const auto f_record2 = file_target_->ParseRecord();
  ASSERT_TRUE(ch_record2.Holds<TbboMsg>());
  ASSERT_TRUE(f_record2.Holds<TbboMsg>());
  const auto& ch_tbbo2 = ch_record2.Get<TbboMsg>();
  const auto& f_tbbo2 = f_record2.Get<TbboMsg>();
  EXPECT_EQ(ch_tbbo2, f_tbbo2);
  EXPECT_EQ(ch_tbbo2.hd.publisher_id, 1);
  EXPECT_EQ(ch_tbbo2.hd.product_id, 5482);
  EXPECT_EQ(ch_tbbo2.hd.ts_event.time_since_epoch().count(),
            1609160400107665963);
  EXPECT_EQ(ch_tbbo2.price, 3720250000000);
  EXPECT_EQ(ch_tbbo2.size, 21);
  EXPECT_EQ(ch_tbbo2.action, 'T');
  EXPECT_EQ(ch_tbbo2.side, 'A');
  EXPECT_EQ(ch_tbbo2.flags, 129);
  EXPECT_EQ(ch_tbbo2.depth, 0);
  EXPECT_EQ(ch_tbbo2.ts_recv.time_since_epoch().count(), 1609160400108142648);
  EXPECT_EQ(ch_tbbo2.ts_in_delta.count(), 20728);
  EXPECT_EQ(ch_tbbo2.sequence, 1170414);
  EXPECT_EQ(ch_tbbo2.booklevel[0].bid_px, 3720250000000);
  EXPECT_EQ(ch_tbbo2.booklevel[0].ask_px, 3720500000000);
  EXPECT_EQ(ch_tbbo2.booklevel[0].bid_sz, 21);
  EXPECT_EQ(ch_tbbo2.booklevel[0].ask_sz, 22);
  EXPECT_EQ(ch_tbbo2.booklevel[0].bid_ct, 13);
  EXPECT_EQ(ch_tbbo2.booklevel[0].ask_ct, 15);
}

TEST_F(DbzParserTests, TestParseTrades) {
  this->ReadFromFile(TEST_BUILD_DIR "/data/test_data.trades.dbz");

  const Metadata ch_metadata = channel_target_.ParseMetadata();
  const Metadata f_metadata = file_target_->ParseMetadata();
  EXPECT_EQ(ch_metadata, f_metadata);
  EXPECT_EQ(ch_metadata.version, 1);
  EXPECT_EQ(ch_metadata.dataset, dataset::kGlbxMdp3);
  EXPECT_EQ(ch_metadata.schema, Schema::Trades);
  EXPECT_EQ(ch_metadata.start.time_since_epoch().count(), 1609160400000000000);
  EXPECT_EQ(ch_metadata.end.time_since_epoch().count(), 1609200000000000000);
  EXPECT_EQ(ch_metadata.limit, 2);
  EXPECT_EQ(ch_metadata.record_count, 2);
  EXPECT_EQ(ch_metadata.stype_in, SType::Native);
  EXPECT_EQ(ch_metadata.stype_out, SType::ProductId);
  EXPECT_EQ(ch_metadata.symbols, std::vector<std::string>{"ESH1"});
  EXPECT_TRUE(ch_metadata.partial.empty());
  EXPECT_TRUE(ch_metadata.not_found.empty());
  AssertMappings(ch_metadata.mappings);

  const auto ch_record1 = channel_target_.ParseRecord();
  const auto f_record1 = file_target_->ParseRecord();
  ASSERT_TRUE(ch_record1.Holds<TradeMsg>());
  ASSERT_TRUE(f_record1.Holds<TradeMsg>());
  const auto& ch_trade1 = ch_record1.Get<TradeMsg>();
  const auto& f_trade1 = f_record1.Get<TradeMsg>();
  EXPECT_EQ(ch_trade1, f_trade1);
  EXPECT_EQ(ch_trade1.hd.publisher_id, 1);
  EXPECT_EQ(ch_trade1.hd.product_id, 5482);
  EXPECT_EQ(ch_trade1.hd.ts_event.time_since_epoch().count(),
            1609160400098821953);
  EXPECT_EQ(ch_trade1.price, 3720250000000);
  EXPECT_EQ(ch_trade1.size, 5);
  EXPECT_EQ(ch_trade1.action, 'T');
  EXPECT_EQ(ch_trade1.side, 'A');
  EXPECT_EQ(ch_trade1.flags, 129);
  EXPECT_EQ(ch_trade1.depth, 0);
  EXPECT_EQ(ch_trade1.ts_recv.time_since_epoch().count(), 1609160400099150057);
  EXPECT_EQ(ch_trade1.ts_in_delta.count(), 19251);
  EXPECT_EQ(ch_trade1.sequence, 1170380);

  const auto ch_record2 = channel_target_.ParseRecord();
  const auto f_record2 = file_target_->ParseRecord();
  ASSERT_TRUE(ch_record2.Holds<TradeMsg>());
  ASSERT_TRUE(f_record2.Holds<TradeMsg>());
  const auto& ch_trade2 = ch_record2.Get<TradeMsg>();
  const auto& f_trade2 = f_record2.Get<TradeMsg>();
  EXPECT_EQ(ch_trade2, f_trade2);
  EXPECT_EQ(ch_trade2.hd.publisher_id, 1);
  EXPECT_EQ(ch_trade2.hd.product_id, 5482);
  EXPECT_EQ(ch_trade2.hd.ts_event.time_since_epoch().count(),
            1609160400107665963);
  EXPECT_EQ(ch_trade2.price, 3720250000000);
  EXPECT_EQ(ch_trade2.size, 21);
  EXPECT_EQ(ch_trade2.action, 'T');
  EXPECT_EQ(ch_trade2.side, 'A');
  EXPECT_EQ(ch_trade2.flags, 129);
  EXPECT_EQ(ch_trade2.depth, 0);
  EXPECT_EQ(ch_trade2.ts_recv.time_since_epoch().count(), 1609160400108142648);
  EXPECT_EQ(ch_trade2.ts_in_delta.count(), 20728);
  EXPECT_EQ(ch_trade2.sequence, 1170414);
}

TEST_F(DbzParserTests, TestParseOhlcv1H) {
  this->ReadFromFile(TEST_BUILD_DIR "/data/test_data.ohlcv-1h.dbz");

  const Metadata ch_metadata = channel_target_.ParseMetadata();
  const Metadata f_metadata = file_target_->ParseMetadata();
  EXPECT_EQ(ch_metadata, f_metadata);
  EXPECT_EQ(ch_metadata.version, 1);
  EXPECT_EQ(ch_metadata.dataset, dataset::kGlbxMdp3);
  EXPECT_EQ(ch_metadata.schema, Schema::Ohlcv1H);
  EXPECT_EQ(ch_metadata.start.time_since_epoch().count(), 1609160400000000000);
  EXPECT_EQ(ch_metadata.end.time_since_epoch().count(), 1609200000000000000);
  EXPECT_EQ(ch_metadata.limit, 2);
  EXPECT_EQ(ch_metadata.record_count, 2);
  EXPECT_EQ(ch_metadata.stype_in, SType::Native);
  EXPECT_EQ(ch_metadata.stype_out, SType::ProductId);
  EXPECT_EQ(ch_metadata.symbols, std::vector<std::string>{"ESH1"});
  EXPECT_TRUE(ch_metadata.partial.empty());
  EXPECT_TRUE(ch_metadata.not_found.empty());
  AssertMappings(ch_metadata.mappings);

  const auto ch_record1 = channel_target_.ParseRecord();
  const auto f_record1 = file_target_->ParseRecord();
  ASSERT_TRUE(ch_record1.Holds<OhlcvMsg>());
  ASSERT_TRUE(f_record1.Holds<OhlcvMsg>());
  const auto& ch_ohlcv1 = ch_record1.Get<OhlcvMsg>();
  const auto& f_ohlcv1 = f_record1.Get<OhlcvMsg>();
  EXPECT_EQ(ch_ohlcv1, f_ohlcv1);
  EXPECT_EQ(ch_ohlcv1.hd.publisher_id, 1);
  EXPECT_EQ(ch_ohlcv1.hd.product_id, 5482);
  EXPECT_EQ(ch_ohlcv1.hd.ts_event.time_since_epoch().count(),
            1609160400000000000);
  EXPECT_EQ(ch_ohlcv1.open, 372025000000000);
  EXPECT_EQ(ch_ohlcv1.high, 372350000000000);
  EXPECT_EQ(ch_ohlcv1.low, 372025000000000);
  EXPECT_EQ(ch_ohlcv1.close, 372225000000000);
  EXPECT_EQ(ch_ohlcv1.volume, 9385);

  const auto ch_record2 = channel_target_.ParseRecord();
  const auto f_record2 = file_target_->ParseRecord();
  ASSERT_TRUE(ch_record2.Holds<OhlcvMsg>());
  ASSERT_TRUE(f_record2.Holds<OhlcvMsg>());
  const auto& ch_ohlcv2 = ch_record2.Get<OhlcvMsg>();
  const auto& f_ohlcv2 = f_record2.Get<OhlcvMsg>();
  EXPECT_EQ(ch_ohlcv2, f_ohlcv2);
  EXPECT_EQ(ch_ohlcv2.hd.publisher_id, 1);
  EXPECT_EQ(ch_ohlcv2.hd.product_id, 5482);
  EXPECT_EQ(ch_ohlcv2.hd.ts_event.time_since_epoch().count(),
            1609164000000000000);
  EXPECT_EQ(ch_ohlcv2.open, 372225000000000);
  EXPECT_EQ(ch_ohlcv2.high, 372450000000000);
  EXPECT_EQ(ch_ohlcv2.low, 371600000000000);
  EXPECT_EQ(ch_ohlcv2.close, 371950000000000);
  EXPECT_EQ(ch_ohlcv2.volume, 112698);
}

TEST_F(DbzParserTests, TestParseOhlcv1M) {
  this->ReadFromFile(TEST_BUILD_DIR "/data/test_data.ohlcv-1m.dbz");

  const Metadata ch_metadata = channel_target_.ParseMetadata();
  const Metadata f_metadata = file_target_->ParseMetadata();
  EXPECT_EQ(ch_metadata, f_metadata);
  EXPECT_EQ(ch_metadata.version, 1);
  EXPECT_EQ(ch_metadata.dataset, dataset::kGlbxMdp3);
  EXPECT_EQ(ch_metadata.schema, Schema::Ohlcv1M);
  EXPECT_EQ(ch_metadata.start.time_since_epoch().count(), 1609160400000000000);
  EXPECT_EQ(ch_metadata.end.time_since_epoch().count(), 1609200000000000000);
  EXPECT_EQ(ch_metadata.limit, 2);
  EXPECT_EQ(ch_metadata.record_count, 2);
  EXPECT_EQ(ch_metadata.stype_in, SType::Native);
  EXPECT_EQ(ch_metadata.stype_out, SType::ProductId);
  EXPECT_EQ(ch_metadata.symbols, std::vector<std::string>{"ESH1"});
  EXPECT_TRUE(ch_metadata.partial.empty());
  EXPECT_TRUE(ch_metadata.not_found.empty());
  AssertMappings(ch_metadata.mappings);

  const auto ch_record1 = channel_target_.ParseRecord();
  const auto f_record1 = file_target_->ParseRecord();
  ASSERT_TRUE(ch_record1.Holds<OhlcvMsg>());
  ASSERT_TRUE(f_record1.Holds<OhlcvMsg>());
  const auto& ch_ohlcv1 = ch_record1.Get<OhlcvMsg>();
  const auto& f_ohlcv1 = f_record1.Get<OhlcvMsg>();
  EXPECT_EQ(ch_ohlcv1, f_ohlcv1);
  EXPECT_EQ(ch_ohlcv1.hd.publisher_id, 1);
  EXPECT_EQ(ch_ohlcv1.hd.product_id, 5482);
  EXPECT_EQ(ch_ohlcv1.hd.ts_event.time_since_epoch().count(),
            1609160400000000000);
  EXPECT_EQ(ch_ohlcv1.open, 372025000000000);
  EXPECT_EQ(ch_ohlcv1.high, 372150000000000);
  EXPECT_EQ(ch_ohlcv1.low, 372025000000000);
  EXPECT_EQ(ch_ohlcv1.close, 372100000000000);
  EXPECT_EQ(ch_ohlcv1.volume, 353);

  const auto ch_record2 = channel_target_.ParseRecord();
  const auto f_record2 = file_target_->ParseRecord();
  ASSERT_TRUE(ch_record2.Holds<OhlcvMsg>());
  ASSERT_TRUE(f_record2.Holds<OhlcvMsg>());
  const auto& ch_ohlcv2 = ch_record2.Get<OhlcvMsg>();
  const auto& f_ohlcv2 = f_record2.Get<OhlcvMsg>();
  EXPECT_EQ(ch_ohlcv2, f_ohlcv2);
  EXPECT_EQ(ch_ohlcv2.hd.publisher_id, 1);
  EXPECT_EQ(ch_ohlcv2.hd.product_id, 5482);
  EXPECT_EQ(ch_ohlcv2.hd.ts_event.time_since_epoch().count(),
            1609160460000000000);
  EXPECT_EQ(ch_ohlcv2.open, 372100000000000);
  EXPECT_EQ(ch_ohlcv2.high, 372150000000000);
  EXPECT_EQ(ch_ohlcv2.low, 372100000000000);
  EXPECT_EQ(ch_ohlcv2.close, 372150000000000);
  EXPECT_EQ(ch_ohlcv2.volume, 152);
}

TEST_F(DbzParserTests, TestParseOhlcv1S) {
  this->ReadFromFile(TEST_BUILD_DIR "/data/test_data.ohlcv-1s.dbz");

  const Metadata ch_metadata = channel_target_.ParseMetadata();
  const Metadata f_metadata = file_target_->ParseMetadata();
  EXPECT_EQ(ch_metadata, f_metadata);
  EXPECT_EQ(ch_metadata.version, 1);
  EXPECT_EQ(ch_metadata.dataset, dataset::kGlbxMdp3);
  EXPECT_EQ(ch_metadata.schema, Schema::Ohlcv1S);
  EXPECT_EQ(ch_metadata.start.time_since_epoch().count(), 1609160400000000000);
  EXPECT_EQ(ch_metadata.end.time_since_epoch().count(), 1609200000000000000);
  EXPECT_EQ(ch_metadata.limit, 2);
  EXPECT_EQ(ch_metadata.record_count, 2);
  EXPECT_EQ(ch_metadata.stype_in, SType::Native);
  EXPECT_EQ(ch_metadata.stype_out, SType::ProductId);
  EXPECT_EQ(ch_metadata.symbols, std::vector<std::string>{"ESH1"});
  EXPECT_TRUE(ch_metadata.partial.empty());
  EXPECT_TRUE(ch_metadata.not_found.empty());
  AssertMappings(ch_metadata.mappings);

  const auto ch_record1 = channel_target_.ParseRecord();
  const auto f_record1 = file_target_->ParseRecord();
  ASSERT_TRUE(ch_record1.Holds<OhlcvMsg>());
  ASSERT_TRUE(f_record1.Holds<OhlcvMsg>());
  const auto& ch_ohlcv1 = ch_record1.Get<OhlcvMsg>();
  const auto& f_ohlcv1 = f_record1.Get<OhlcvMsg>();
  EXPECT_EQ(ch_ohlcv1, f_ohlcv1);
  EXPECT_EQ(ch_ohlcv1.hd.publisher_id, 1);
  EXPECT_EQ(ch_ohlcv1.hd.product_id, 5482);
  EXPECT_EQ(ch_ohlcv1.hd.ts_event.time_since_epoch().count(),
            1609160400000000000);
  EXPECT_EQ(ch_ohlcv1.open, 372025000000000);
  EXPECT_EQ(ch_ohlcv1.high, 372050000000000);
  EXPECT_EQ(ch_ohlcv1.low, 372025000000000);
  EXPECT_EQ(ch_ohlcv1.close, 372050000000000);
  EXPECT_EQ(ch_ohlcv1.volume, 57);

  const auto ch_record2 = channel_target_.ParseRecord();
  const auto f_record2 = file_target_->ParseRecord();
  ASSERT_TRUE(ch_record2.Holds<OhlcvMsg>());
  ASSERT_TRUE(f_record2.Holds<OhlcvMsg>());
  const auto& ch_ohlcv2 = ch_record2.Get<OhlcvMsg>();
  const auto& f_ohlcv2 = f_record2.Get<OhlcvMsg>();
  EXPECT_EQ(ch_ohlcv2, f_ohlcv2);
  EXPECT_EQ(ch_ohlcv2.hd.publisher_id, 1);
  EXPECT_EQ(ch_ohlcv2.hd.product_id, 5482);
  EXPECT_EQ(ch_ohlcv2.hd.ts_event.time_since_epoch().count(),
            1609160401000000000);
  EXPECT_EQ(ch_ohlcv2.open, 372050000000000);
  EXPECT_EQ(ch_ohlcv2.high, 372050000000000);
  EXPECT_EQ(ch_ohlcv2.low, 372050000000000);
  EXPECT_EQ(ch_ohlcv2.close, 372050000000000);
  EXPECT_EQ(ch_ohlcv2.volume, 13);
}

TEST_F(DbzParserTests, TestParseDefinition) {
  this->ReadFromFile(TEST_BUILD_DIR "/data/test_data.definition.dbz");

  const Metadata ch_metadata = channel_target_.ParseMetadata();
  const Metadata f_metadata = file_target_->ParseMetadata();
  EXPECT_EQ(ch_metadata, f_metadata);
  EXPECT_EQ(ch_metadata.version, 1);
  EXPECT_EQ(ch_metadata.dataset, dataset::kXnasItch);
  EXPECT_EQ(ch_metadata.schema, Schema::Definition);
  EXPECT_EQ(ch_metadata.start.time_since_epoch().count(), 1664841600000000000);
  EXPECT_EQ(ch_metadata.end.time_since_epoch().count(), 1672790400000000000);
  EXPECT_EQ(ch_metadata.limit, 0);
  EXPECT_EQ(ch_metadata.record_count, 2);
  EXPECT_EQ(ch_metadata.stype_in, SType::Native);
  EXPECT_EQ(ch_metadata.stype_out, SType::ProductId);
  EXPECT_EQ(ch_metadata.symbols, std::vector<std::string>{"MSFT"});
  EXPECT_TRUE(ch_metadata.partial.empty());
  EXPECT_TRUE(ch_metadata.not_found.empty());
  EXPECT_EQ(ch_metadata.mappings.size(), 1);
  const auto& mapping = ch_metadata.mappings.at(0);
  EXPECT_EQ(mapping.native, "MSFT");
  ASSERT_EQ(mapping.intervals.size(), 20);
  const auto& interval = mapping.intervals.at(0);
  EXPECT_EQ(interval.symbol, "7358");
  EXPECT_EQ(interval.start_date, 20221004);
  EXPECT_EQ(interval.end_date, 20221205);

  const auto ch_record1 = channel_target_.ParseRecord();
  const auto f_record1 = file_target_->ParseRecord();
  ASSERT_TRUE(ch_record1.Holds<InstrumentDefMsg>());
  ASSERT_TRUE(f_record1.Holds<InstrumentDefMsg>());
  const auto& ch_def1 = ch_record1.Get<InstrumentDefMsg>();
  const auto& f_def1 = f_record1.Get<InstrumentDefMsg>();
  EXPECT_EQ(ch_def1, f_def1);
  EXPECT_STREQ(ch_def1.exchange.data(), "XNAS");
  EXPECT_STREQ(ch_def1.symbol.data(), "MSFT");
  EXPECT_EQ(ch_def1.security_update_action, 'A');
  EXPECT_EQ(ch_def1.min_lot_size_round_lot, 100);

  const auto ch_record2 = channel_target_.ParseRecord();
  const auto f_record2 = file_target_->ParseRecord();
  ASSERT_TRUE(ch_record2.Holds<InstrumentDefMsg>());
  ASSERT_TRUE(f_record2.Holds<InstrumentDefMsg>());
  const auto& ch_def2 = ch_record2.Get<InstrumentDefMsg>();
  const auto& f_def2 = f_record2.Get<InstrumentDefMsg>();
  EXPECT_EQ(ch_def2, f_def2);
  EXPECT_STREQ(ch_def2.exchange.data(), "XNAS");
  EXPECT_STREQ(ch_def2.symbol.data(), "MSFT");
  EXPECT_EQ(ch_def2.security_update_action, 'A');
  EXPECT_EQ(ch_def2.min_lot_size_round_lot, 100);
}
}  // namespace test
}  // namespace databento
