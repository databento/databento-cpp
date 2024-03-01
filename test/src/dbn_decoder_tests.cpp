#include <gtest/gtest.h>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>  // ifstream
#include <ios>      // streamsize, ios::binary, ios::ate
#include <memory>

#include "databento/compat.hpp"
#include "databento/constants.hpp"
#include "databento/datetime.hpp"
#include "databento/dbn.hpp"
#include "databento/dbn_decoder.hpp"
#include "databento/detail/file_stream.hpp"
#include "databento/detail/scoped_thread.hpp"
#include "databento/detail/shared_channel.hpp"
#include "databento/enums.hpp"
#include "databento/exceptions.hpp"
#include "databento/ireadable.hpp"
#include "databento/record.hpp"
#include "databento/with_ts_out.hpp"

namespace databento {
namespace test {
class DbnDecoderTests : public testing::Test {
 public:
  detail::SharedChannel channel_;
  std::unique_ptr<DbnDecoder> file_target_;
  std::unique_ptr<DbnDecoder> channel_target_;
  detail::ScopedThread write_thread_;

  void ReadFromFile(const std::string& schema_str, const std::string& extension,
                    std::uint8_t version) {
    ReadFromFile(schema_str, extension, version, VersionUpgradePolicy::AsIs);
  }

  void ReadFromFile(const std::string& schema_str, const std::string& extension,
                    std::uint8_t version, VersionUpgradePolicy upgrade_policy) {
    const char* version_str = version == 1 ? ".v1" : "";
    const std::string file_path = TEST_BUILD_DIR "/data/test_data." +
                                  schema_str + version_str + extension;
    // Channel setup
    write_thread_ = detail::ScopedThread{[this, file_path] {
      std::ifstream input_file{file_path, std::ios::binary | std::ios::ate};
      ASSERT_TRUE(input_file.good());
      const auto size = static_cast<std::size_t>(input_file.tellg());
      input_file.seekg(0, std::ios::beg);
      std::vector<char> buffer(size);
      input_file.read(buffer.data(), static_cast<std::streamsize>(size));
      ASSERT_EQ(input_file.gcount(), size);
      channel_.Write(reinterpret_cast<const std::uint8_t*>(buffer.data()),
                     size);
      channel_.Finish();
    }};
    channel_target_.reset(new DbnDecoder{
        std::unique_ptr<IReadable>{new detail::SharedChannel{channel_}},
        upgrade_policy});
    // File setup
    file_target_.reset(new DbnDecoder{
        std::unique_ptr<IReadable>{new detail::FileStream{file_path}},
        upgrade_policy});
  }

  static void AssertMappings(const std::vector<SymbolMapping>& mappings) {
    ASSERT_EQ(mappings.size(), 1);
    const auto& mapping = mappings.at(0);
    EXPECT_EQ(mapping.raw_symbol, "ESH1");
    ASSERT_EQ(mapping.intervals.size(), 1);
    const auto& interval = mapping.intervals.at(0);
    EXPECT_EQ(interval.symbol, "5482");
    EXPECT_EQ(interval.start_date, 20201228);
    EXPECT_EQ(interval.end_date, 20201229);
  }
};

template <typename D>
void AssertDefEq(const Record* ch_record, const Record* f_record) {
  ASSERT_TRUE(ch_record->Holds<D>());
  ASSERT_TRUE(f_record->Holds<D>());
  const auto& ch_def = ch_record->Get<D>();
  const auto& f_def = f_record->Get<D>();
  EXPECT_EQ(ch_def, f_def);
  EXPECT_STREQ(ch_def.Exchange(), "XNAS");
  EXPECT_STREQ(ch_def.RawSymbol(), "MSFT");
  EXPECT_EQ(ch_def.security_update_action, SecurityUpdateAction::Add);
  EXPECT_EQ(ch_def.min_lot_size_round_lot, 100);
  EXPECT_EQ(ch_def.instrument_class, InstrumentClass::Stock);
  EXPECT_EQ(ch_def.strike_price, kUndefPrice);
}

TEST_F(DbnDecoderTests, TestDecodeDbz) {
  try {
    ReadFromFile("mbo", ".dbz", 0);

    FAIL() << "Decoding DBZ should throw";
  } catch (const DbnResponseError& err) {
    ASSERT_STREQ(err.what(),
                 "Legacy DBZ encoding is not supported. Please use the dbn CLI "
                 "tool to convert it to DBN.");
  }
}

TEST_F(DbnDecoderTests, TestDecodeDefinitionUpgrade) {
  ReadFromFile("definition", ".dbn", 1, VersionUpgradePolicy::Upgrade);

  const Metadata ch_metadata = channel_target_->DecodeMetadata();
  const Metadata f_metadata = file_target_->DecodeMetadata();
  EXPECT_EQ(ch_metadata, f_metadata);
  EXPECT_EQ(ch_metadata.version, 1);
  EXPECT_EQ(ch_metadata.dataset, dataset::kXnasItch);
  EXPECT_EQ(ch_metadata.schema, Schema::Definition);
  EXPECT_EQ(ch_metadata.start.time_since_epoch().count(), 1633305600000000000);
  EXPECT_EQ(ch_metadata.end.time_since_epoch().count(), 1641254400000000000);
  EXPECT_EQ(ch_metadata.limit, 2);
  EXPECT_EQ(ch_metadata.stype_in, SType::RawSymbol);
  EXPECT_EQ(ch_metadata.stype_out, SType::InstrumentId);
  EXPECT_EQ(ch_metadata.symbols, std::vector<std::string>{"MSFT"});
  EXPECT_TRUE(ch_metadata.partial.empty());
  EXPECT_TRUE(ch_metadata.not_found.empty());
  EXPECT_EQ(ch_metadata.mappings.size(), 1);
  const auto& mapping = ch_metadata.mappings.at(0);
  EXPECT_EQ(mapping.raw_symbol, "MSFT");
  ASSERT_EQ(mapping.intervals.size(), 62);
  const auto& interval = mapping.intervals.at(0);
  EXPECT_EQ(interval.symbol, "6819");
  EXPECT_EQ(interval.start_date, 20211004);
  EXPECT_EQ(interval.end_date, 20211005);

  const auto ch_record1 = channel_target_->DecodeRecord();
  const auto f_record1 = file_target_->DecodeRecord();
  ASSERT_NE(ch_record1, nullptr);
  ASSERT_NE(f_record1, nullptr);

  const auto ch_record2 = channel_target_->DecodeRecord();
  const auto f_record2 = file_target_->DecodeRecord();
  ASSERT_NE(ch_record2, nullptr);
  ASSERT_NE(f_record2, nullptr);
  AssertDefEq<InstrumentDefMsgV2>(ch_record1, f_record1);
  AssertDefEq<InstrumentDefMsgV2>(ch_record2, f_record2);
}

TEST_F(DbnDecoderTests, TestUpgradeSymbolMappingWithTsOut) {
  SymbolMappingMsgV1 sym_map{
      {sizeof(SymbolMappingMsgV1) / RecordHeader::kLengthMultiplier,
       RType::SymbolMapping, 0, 1, UnixNanos{std::chrono::nanoseconds{2}}},
      {"ES.c.0"},
      {"ESH4"},
      {},
      {},
      {}};
  WithTsOut<SymbolMappingMsgV1> orig{
      sym_map, UnixNanos{std::chrono::system_clock::now()}};
  std::array<std::uint8_t, kMaxRecordLen> compat_buffer{};
  const auto res =
      DbnDecoder::DecodeRecordCompat(1, VersionUpgradePolicy::Upgrade, true,
                                     &compat_buffer, Record{&orig.rec.hd});
  const auto& upgraded = res.Get<WithTsOut<SymbolMappingMsgV2>>();
  ASSERT_EQ(orig.ts_out, upgraded.ts_out);
  ASSERT_STREQ(orig.rec.STypeInSymbol(), upgraded.rec.STypeInSymbol());
  ASSERT_STREQ(orig.rec.STypeOutSymbol(), upgraded.rec.STypeOutSymbol());
  // `length` properly set
  ASSERT_EQ(upgraded.rec.hd.Size(), sizeof(upgraded));
  // used compat buffer
  ASSERT_EQ(reinterpret_cast<const std::uint8_t*>(&upgraded),
            compat_buffer.data());
}

TEST_F(DbnDecoderTests, TestUpgradeMbp1WithTsOut) {
  WithTsOut<Mbp1Msg> orig{
      Mbp1Msg{{sizeof(Mbp1Msg) / RecordHeader::kLengthMultiplier,
               RType::Mbp1,
               {},
               {},
               {}},
              1'250'000'000,
              {},
              {},
              Side::Ask,
              {},
              {},
              {},
              {},
              {},
              {}},
      {std::chrono::system_clock::now()}};
  std::array<std::uint8_t, kMaxRecordLen> compat_buffer{};
  const auto res =
      DbnDecoder::DecodeRecordCompat(1, VersionUpgradePolicy::Upgrade, true,
                                     &compat_buffer, Record{&orig.rec.hd});
  const auto& upgraded = res.Get<WithTsOut<Mbp1Msg>>();
  // compat buffer unused and pointer unchanged
  ASSERT_EQ(&orig, &upgraded);
}

class DbnDecoderSchemaTests
    : public DbnDecoderTests,
      public testing::WithParamInterface<std::pair<const char*, std::uint8_t>> {
};

INSTANTIATE_TEST_SUITE_P(
    TestFiles, DbnDecoderSchemaTests,
    testing::Values(std::make_pair(".dbn", 1), std::make_pair(".dbn", 2),
                    std::make_pair(".dbn.zst", 1),
                    std::make_pair(".dbn.zst", 2)),
    [](const testing::TestParamInfo<std::pair<const char*, std::uint8_t>>&
           test_info) {
      const auto extension = test_info.param.first;
      const auto version = test_info.param.second;
      const auto size = ::strlen(extension);
      return ::strncmp(extension + size - 3, "zst", 3) == 0
                 ? "ZstdDBNv" + std::to_string(version)
                 : "UncompressedDBNv" + std::to_string(version);
    });

// Expected data for these tests obtained using the `dbn` CLI tool

TEST_P(DbnDecoderSchemaTests, TestDecodeMbo) {
  const auto extension = GetParam().first;
  const auto version = GetParam().second;
  ReadFromFile("mbo", extension, version);

  const Metadata ch_metadata = channel_target_->DecodeMetadata();
  const Metadata f_metadata = file_target_->DecodeMetadata();
  EXPECT_EQ(ch_metadata, f_metadata);
  EXPECT_EQ(ch_metadata.version, version);
  EXPECT_EQ(ch_metadata.dataset, dataset::kGlbxMdp3);
  EXPECT_EQ(ch_metadata.schema, Schema::Mbo);
  EXPECT_EQ(ch_metadata.start.time_since_epoch().count(), 1609160400000000000);
  EXPECT_EQ(ch_metadata.end.time_since_epoch().count(), 1609200000000000000);
  EXPECT_EQ(ch_metadata.limit, 2);
  EXPECT_EQ(ch_metadata.stype_in, SType::RawSymbol);
  EXPECT_EQ(ch_metadata.stype_out, SType::InstrumentId);
  EXPECT_EQ(ch_metadata.symbols, std::vector<std::string>{"ESH1"});
  EXPECT_TRUE(ch_metadata.partial.empty());
  EXPECT_TRUE(ch_metadata.not_found.empty());

  const auto ch_record1 = channel_target_->DecodeRecord();
  const auto f_record1 = file_target_->DecodeRecord();
  ASSERT_NE(ch_record1, nullptr);
  ASSERT_NE(f_record1, nullptr);
  ASSERT_TRUE(ch_record1->Holds<MboMsg>());
  ASSERT_TRUE(f_record1->Holds<MboMsg>());
  const auto& ch_mbo1 = ch_record1->Get<MboMsg>();
  const auto& f_mbo1 = f_record1->Get<MboMsg>();
  EXPECT_EQ(ch_mbo1, f_mbo1);
  EXPECT_EQ(ch_mbo1.hd.publisher_id, 1);
  EXPECT_EQ(ch_mbo1.hd.instrument_id, 5482);
  EXPECT_EQ(ch_mbo1.hd.ts_event.time_since_epoch().count(),
            1609160400000429831);
  EXPECT_EQ(ch_mbo1.order_id, 647784973705);
  EXPECT_EQ(ch_mbo1.price, 3722750000000);
  EXPECT_EQ(ch_mbo1.size, 1);
  EXPECT_EQ(ch_mbo1.flags, 128);
  EXPECT_EQ(ch_mbo1.channel_id, 0);
  EXPECT_EQ(ch_mbo1.action, Action::Cancel);
  EXPECT_EQ(ch_mbo1.side, Side::Ask);
  EXPECT_EQ(ch_mbo1.ts_recv.time_since_epoch().count(), 1609160400000704060);
  EXPECT_EQ(ch_mbo1.ts_in_delta.count(), 22993);
  EXPECT_EQ(ch_mbo1.sequence, 1170352);

  const auto ch_record2 = channel_target_->DecodeRecord();
  const auto f_record2 = file_target_->DecodeRecord();
  ASSERT_NE(ch_record2, nullptr);
  ASSERT_NE(f_record2, nullptr);
  ASSERT_TRUE(ch_record2->Holds<MboMsg>());
  ASSERT_TRUE(f_record2->Holds<MboMsg>());
  const auto& ch_mbo2 = ch_record2->Get<MboMsg>();
  const auto& f_mbo2 = f_record2->Get<MboMsg>();
  EXPECT_EQ(ch_mbo2, f_mbo2);
  EXPECT_EQ(ch_mbo2.hd.publisher_id, 1);
  EXPECT_EQ(ch_mbo2.hd.instrument_id, 5482);
  EXPECT_EQ(ch_mbo2.hd.ts_event.time_since_epoch().count(),
            1609160400000431665);
  EXPECT_EQ(ch_mbo2.order_id, 647784973631);
  EXPECT_EQ(ch_mbo2.price, 3723000000000);
  EXPECT_EQ(ch_mbo2.size, 1);
  EXPECT_EQ(ch_mbo2.flags, 128);
  EXPECT_EQ(ch_mbo2.channel_id, 0);
  EXPECT_EQ(ch_mbo2.action, Action::Cancel);
  EXPECT_EQ(ch_mbo2.side, Side::Ask);
  EXPECT_EQ(ch_mbo2.ts_recv.time_since_epoch().count(), 1609160400000711344);
  EXPECT_EQ(ch_mbo2.ts_in_delta.count(), 19621);
  EXPECT_EQ(ch_mbo2.sequence, 1170353);
}

TEST_P(DbnDecoderSchemaTests, TestDecodeMbp1) {
  const auto extension = GetParam().first;
  const auto version = GetParam().second;
  ReadFromFile("mbp-1", extension, version);

  const Metadata ch_metadata = channel_target_->DecodeMetadata();
  const Metadata f_metadata = file_target_->DecodeMetadata();
  EXPECT_EQ(ch_metadata, f_metadata);
  EXPECT_EQ(ch_metadata.version, version);
  EXPECT_EQ(ch_metadata.dataset, dataset::kGlbxMdp3);
  EXPECT_EQ(ch_metadata.schema, Schema::Mbp1);
  EXPECT_EQ(ch_metadata.start.time_since_epoch().count(), 1609160400000000000);
  EXPECT_EQ(ch_metadata.end.time_since_epoch().count(), 1609200000000000000);
  EXPECT_EQ(ch_metadata.limit, 2);
  EXPECT_EQ(ch_metadata.stype_in, SType::RawSymbol);
  EXPECT_EQ(ch_metadata.stype_out, SType::InstrumentId);
  EXPECT_EQ(ch_metadata.symbols, std::vector<std::string>{"ESH1"});
  EXPECT_TRUE(ch_metadata.partial.empty());
  EXPECT_TRUE(ch_metadata.not_found.empty());
  AssertMappings(ch_metadata.mappings);

  const auto ch_record1 = channel_target_->DecodeRecord();
  const auto f_record1 = file_target_->DecodeRecord();
  ASSERT_NE(ch_record1, nullptr);
  ASSERT_NE(f_record1, nullptr);
  ASSERT_TRUE(ch_record1->Holds<Mbp1Msg>());
  ASSERT_TRUE(f_record1->Holds<Mbp1Msg>());
  const auto& ch_mbp1 = ch_record1->Get<Mbp1Msg>();
  const auto& f_mbp1 = f_record1->Get<Mbp1Msg>();
  EXPECT_EQ(ch_mbp1, f_mbp1);
  EXPECT_EQ(ch_mbp1.hd.publisher_id, 1);
  EXPECT_EQ(ch_mbp1.hd.instrument_id, 5482);
  EXPECT_EQ(ch_mbp1.hd.ts_event.time_since_epoch().count(),
            1609160400006001487);
  EXPECT_EQ(ch_mbp1.price, 3720500000000);
  EXPECT_EQ(ch_mbp1.size, 1);
  EXPECT_EQ(ch_mbp1.action, Action::Add);
  EXPECT_EQ(ch_mbp1.side, Side::Ask);
  EXPECT_EQ(ch_mbp1.flags, 128);
  EXPECT_EQ(ch_mbp1.depth, 0);
  EXPECT_EQ(ch_mbp1.ts_recv.time_since_epoch().count(), 1609160400006136329);
  EXPECT_EQ(ch_mbp1.ts_in_delta.count(), 17214);
  EXPECT_EQ(ch_mbp1.sequence, 1170362);
  EXPECT_EQ(ch_mbp1.levels[0].bid_px, 3720250000000);
  EXPECT_EQ(ch_mbp1.levels[0].ask_px, 3720500000000);
  EXPECT_EQ(ch_mbp1.levels[0].bid_sz, 24);
  EXPECT_EQ(ch_mbp1.levels[0].ask_sz, 11);
  EXPECT_EQ(ch_mbp1.levels[0].bid_ct, 15);
  EXPECT_EQ(ch_mbp1.levels[0].ask_ct, 9);

  const auto ch_record2 = channel_target_->DecodeRecord();
  const auto f_record2 = file_target_->DecodeRecord();
  ASSERT_NE(ch_record2, nullptr);
  ASSERT_NE(f_record2, nullptr);
  ASSERT_TRUE(ch_record2->Holds<Mbp1Msg>());
  ASSERT_TRUE(f_record2->Holds<Mbp1Msg>());
  const auto& ch_mbp2 = ch_record2->Get<Mbp1Msg>();
  const auto& f_mbp2 = f_record2->Get<Mbp1Msg>();
  EXPECT_EQ(ch_mbp2, f_mbp2);
  EXPECT_EQ(ch_mbp2.hd.publisher_id, 1);
  EXPECT_EQ(ch_mbp2.hd.instrument_id, 5482);
  EXPECT_EQ(ch_mbp2.hd.ts_event.time_since_epoch().count(),
            1609160400006146661);
  EXPECT_EQ(ch_mbp2.price, 3720500000000);
  EXPECT_EQ(ch_mbp2.size, 1);
  EXPECT_EQ(ch_mbp2.action, Action::Add);
  EXPECT_EQ(ch_mbp2.side, Side::Ask);
  EXPECT_EQ(ch_mbp2.flags, 128);
  EXPECT_EQ(ch_mbp2.depth, 0);
  EXPECT_EQ(ch_mbp2.ts_recv.time_since_epoch().count(), 1609160400006246513);
  EXPECT_EQ(ch_mbp2.ts_in_delta.count(), 18858);
  EXPECT_EQ(ch_mbp2.sequence, 1170364);
  EXPECT_EQ(ch_mbp2.levels[0].bid_px, 3720250000000);
  EXPECT_EQ(ch_mbp2.levels[0].ask_px, 3720500000000);
  EXPECT_EQ(ch_mbp2.levels[0].bid_sz, 24);
  EXPECT_EQ(ch_mbp2.levels[0].ask_sz, 12);
  EXPECT_EQ(ch_mbp2.levels[0].bid_ct, 15);
  EXPECT_EQ(ch_mbp2.levels[0].ask_ct, 10);
}

TEST_P(DbnDecoderSchemaTests, TestDecodeMbp10) {
  const auto extension = GetParam().first;
  const auto version = GetParam().second;
  ReadFromFile("mbp-10", extension, version);

  const Metadata ch_metadata = channel_target_->DecodeMetadata();
  const Metadata f_metadata = file_target_->DecodeMetadata();
  EXPECT_EQ(ch_metadata, f_metadata);
  EXPECT_EQ(ch_metadata.version, version);
  EXPECT_EQ(ch_metadata.dataset, dataset::kGlbxMdp3);
  EXPECT_EQ(ch_metadata.schema, Schema::Mbp10);
  EXPECT_EQ(ch_metadata.start.time_since_epoch().count(), 1609160400000000000);
  EXPECT_EQ(ch_metadata.end.time_since_epoch().count(), 1609200000000000000);
  EXPECT_EQ(ch_metadata.limit, 2);
  EXPECT_EQ(ch_metadata.stype_in, SType::RawSymbol);
  EXPECT_EQ(ch_metadata.stype_out, SType::InstrumentId);
  EXPECT_EQ(ch_metadata.symbols, std::vector<std::string>{"ESH1"});
  EXPECT_TRUE(ch_metadata.partial.empty());
  EXPECT_TRUE(ch_metadata.not_found.empty());
  AssertMappings(ch_metadata.mappings);

  const auto ch_record1 = channel_target_->DecodeRecord();
  const auto f_record1 = file_target_->DecodeRecord();
  ASSERT_NE(ch_record1, nullptr);
  ASSERT_NE(f_record1, nullptr);
  ASSERT_TRUE(ch_record1->Holds<Mbp10Msg>());
  ASSERT_TRUE(f_record1->Holds<Mbp10Msg>());
  const auto& ch_mbp1 = ch_record1->Get<Mbp10Msg>();
  const auto& f_mbp1 = f_record1->Get<Mbp10Msg>();
  EXPECT_EQ(ch_mbp1, f_mbp1);
  EXPECT_EQ(ch_mbp1.hd.publisher_id, 1);
  EXPECT_EQ(ch_mbp1.hd.instrument_id, 5482);
  EXPECT_EQ(ch_mbp1.hd.ts_event.time_since_epoch().count(),
            1609160400000429831);
  EXPECT_EQ(ch_mbp1.price, 3722750000000);
  EXPECT_EQ(ch_mbp1.size, 1);
  EXPECT_EQ(ch_mbp1.action, Action::Cancel);
  EXPECT_EQ(ch_mbp1.side, Side::Ask);
  EXPECT_EQ(ch_mbp1.flags, 128);
  EXPECT_EQ(ch_mbp1.depth, 9);
  EXPECT_EQ(ch_mbp1.ts_recv.time_since_epoch().count(), 1609160400000704060);
  EXPECT_EQ(ch_mbp1.ts_in_delta.count(), 22993);
  EXPECT_EQ(ch_mbp1.sequence, 1170352);
  EXPECT_EQ(ch_mbp1.levels[0].bid_px, 3720250000000);
  EXPECT_EQ(ch_mbp1.levels[0].ask_px, 3720500000000);
  EXPECT_EQ(ch_mbp1.levels[0].bid_sz, 24);
  EXPECT_EQ(ch_mbp1.levels[0].ask_sz, 10);
  EXPECT_EQ(ch_mbp1.levels[0].bid_ct, 15);
  EXPECT_EQ(ch_mbp1.levels[0].ask_ct, 8);
  EXPECT_EQ(ch_mbp1.levels[1].bid_px, 3720000000000);
  EXPECT_EQ(ch_mbp1.levels[1].ask_px, 3720750000000);
  EXPECT_EQ(ch_mbp1.levels[1].bid_sz, 31);
  EXPECT_EQ(ch_mbp1.levels[1].ask_sz, 34);
  EXPECT_EQ(ch_mbp1.levels[1].bid_ct, 18);
  EXPECT_EQ(ch_mbp1.levels[1].ask_ct, 24);
  EXPECT_EQ(ch_mbp1.levels[2].bid_px, 3719750000000);
  EXPECT_EQ(ch_mbp1.levels[2].ask_px, 3721000000000);
  EXPECT_EQ(ch_mbp1.levels[2].bid_sz, 32);
  EXPECT_EQ(ch_mbp1.levels[2].ask_sz, 39);
  EXPECT_EQ(ch_mbp1.levels[2].bid_ct, 23);
  EXPECT_EQ(ch_mbp1.levels[2].ask_ct, 25);

  const auto ch_record2 = channel_target_->DecodeRecord();
  const auto f_record2 = file_target_->DecodeRecord();
  ASSERT_NE(ch_record2, nullptr);
  ASSERT_NE(f_record2, nullptr);
  ASSERT_TRUE(ch_record2->Holds<Mbp10Msg>());
  ASSERT_TRUE(f_record2->Holds<Mbp10Msg>());
  const auto& ch_mbp2 = ch_record2->Get<Mbp10Msg>();
  const auto& f_mbp2 = f_record2->Get<Mbp10Msg>();
  EXPECT_EQ(ch_mbp2, f_mbp2);
  EXPECT_EQ(ch_mbp2.hd.publisher_id, 1);
  EXPECT_EQ(ch_mbp2.hd.instrument_id, 5482);
  EXPECT_EQ(ch_mbp2.hd.ts_event.time_since_epoch().count(),
            1609160400000435673);
  EXPECT_EQ(ch_mbp2.price, 3720000000000);
  EXPECT_EQ(ch_mbp2.size, 1);
  EXPECT_EQ(ch_mbp2.action, Action::Cancel);
  EXPECT_EQ(ch_mbp2.side, Side::Bid);
  EXPECT_EQ(ch_mbp2.flags, 128);
  EXPECT_EQ(ch_mbp2.depth, 1);
  EXPECT_EQ(ch_mbp2.ts_recv.time_since_epoch().count(), 1609160400000750544);
  EXPECT_EQ(ch_mbp2.ts_in_delta.count(), 20625);
  EXPECT_EQ(ch_mbp2.sequence, 1170356);
  EXPECT_EQ(ch_mbp2.levels[0].bid_px, 3720250000000);
  EXPECT_EQ(ch_mbp2.levels[0].ask_px, 3720500000000);
  EXPECT_EQ(ch_mbp2.levels[0].bid_sz, 24);
  EXPECT_EQ(ch_mbp2.levels[0].ask_sz, 10);
  EXPECT_EQ(ch_mbp2.levels[0].bid_ct, 15);
  EXPECT_EQ(ch_mbp2.levels[0].ask_ct, 8);
  EXPECT_EQ(ch_mbp2.levels[1].bid_px, 3720000000000);
  EXPECT_EQ(ch_mbp2.levels[1].ask_px, 3720750000000);
  EXPECT_EQ(ch_mbp2.levels[1].bid_sz, 30);
  EXPECT_EQ(ch_mbp2.levels[1].ask_sz, 34);
  EXPECT_EQ(ch_mbp2.levels[1].bid_ct, 17);
  EXPECT_EQ(ch_mbp2.levels[1].ask_ct, 24);
  EXPECT_EQ(ch_mbp2.levels[2].bid_px, 3719750000000);
  EXPECT_EQ(ch_mbp2.levels[2].ask_px, 3721000000000);
  EXPECT_EQ(ch_mbp2.levels[2].bid_sz, 32);
  EXPECT_EQ(ch_mbp2.levels[2].ask_sz, 39);
  EXPECT_EQ(ch_mbp2.levels[2].bid_ct, 23);
  EXPECT_EQ(ch_mbp2.levels[2].ask_ct, 25);
}

TEST_P(DbnDecoderSchemaTests, TestDecodeTbbo) {
  const auto extension = GetParam().first;
  const auto version = GetParam().second;
  ReadFromFile("tbbo", extension, version);

  const Metadata ch_metadata = channel_target_->DecodeMetadata();
  const Metadata f_metadata = file_target_->DecodeMetadata();
  EXPECT_EQ(ch_metadata, f_metadata);
  EXPECT_EQ(ch_metadata.version, version);
  EXPECT_EQ(ch_metadata.dataset, dataset::kGlbxMdp3);
  EXPECT_EQ(ch_metadata.schema, Schema::Tbbo);
  EXPECT_EQ(ch_metadata.start.time_since_epoch().count(), 1609160400000000000);
  EXPECT_EQ(ch_metadata.end.time_since_epoch().count(), 1609200000000000000);
  EXPECT_EQ(ch_metadata.limit, 2);
  EXPECT_EQ(ch_metadata.stype_in, SType::RawSymbol);
  EXPECT_EQ(ch_metadata.stype_out, SType::InstrumentId);
  EXPECT_EQ(ch_metadata.symbols, std::vector<std::string>{"ESH1"});
  EXPECT_TRUE(ch_metadata.partial.empty());
  EXPECT_TRUE(ch_metadata.not_found.empty());
  AssertMappings(ch_metadata.mappings);

  const auto ch_record1 = channel_target_->DecodeRecord();
  const auto f_record1 = file_target_->DecodeRecord();
  ASSERT_NE(ch_record1, nullptr);
  ASSERT_NE(f_record1, nullptr);
  ASSERT_TRUE(ch_record1->Holds<TbboMsg>());
  ASSERT_TRUE(f_record1->Holds<TbboMsg>());
  const auto& ch_tbbo1 = ch_record1->Get<TbboMsg>();
  const auto& f_tbbo1 = f_record1->Get<TbboMsg>();
  EXPECT_EQ(ch_tbbo1, f_tbbo1);
  EXPECT_EQ(ch_tbbo1.hd.publisher_id, 1);
  EXPECT_EQ(ch_tbbo1.hd.instrument_id, 5482);
  EXPECT_EQ(ch_tbbo1.hd.ts_event.time_since_epoch().count(),
            1609160400098821953);
  EXPECT_EQ(ch_tbbo1.price, 3720250000000);
  EXPECT_EQ(ch_tbbo1.size, 5);
  EXPECT_EQ(ch_tbbo1.action, Action::Trade);
  EXPECT_EQ(ch_tbbo1.side, Side::Ask);
  EXPECT_EQ(ch_tbbo1.flags, 129);
  EXPECT_EQ(ch_tbbo1.depth, 0);
  EXPECT_EQ(ch_tbbo1.ts_recv.time_since_epoch().count(), 1609160400099150057);
  EXPECT_EQ(ch_tbbo1.ts_in_delta.count(), 19251);
  EXPECT_EQ(ch_tbbo1.sequence, 1170380);
  EXPECT_EQ(ch_tbbo1.levels[0].bid_px, 3720250000000);
  EXPECT_EQ(ch_tbbo1.levels[0].ask_px, 3720500000000);
  EXPECT_EQ(ch_tbbo1.levels[0].bid_sz, 26);
  EXPECT_EQ(ch_tbbo1.levels[0].ask_sz, 7);
  EXPECT_EQ(ch_tbbo1.levels[0].bid_ct, 16);
  EXPECT_EQ(ch_tbbo1.levels[0].ask_ct, 6);

  const auto ch_record2 = channel_target_->DecodeRecord();
  const auto f_record2 = file_target_->DecodeRecord();
  ASSERT_NE(ch_record2, nullptr);
  ASSERT_NE(f_record2, nullptr);
  ASSERT_TRUE(ch_record2->Holds<TbboMsg>());
  ASSERT_TRUE(f_record2->Holds<TbboMsg>());
  const auto& ch_tbbo2 = ch_record2->Get<TbboMsg>();
  const auto& f_tbbo2 = f_record2->Get<TbboMsg>();
  EXPECT_EQ(ch_tbbo2, f_tbbo2);
  EXPECT_EQ(ch_tbbo2.hd.publisher_id, 1);
  EXPECT_EQ(ch_tbbo2.hd.instrument_id, 5482);
  EXPECT_EQ(ch_tbbo2.hd.ts_event.time_since_epoch().count(),
            1609160400107665963);
  EXPECT_EQ(ch_tbbo2.price, 3720250000000);
  EXPECT_EQ(ch_tbbo2.size, 21);
  EXPECT_EQ(ch_tbbo2.action, Action::Trade);
  EXPECT_EQ(ch_tbbo2.side, Side::Ask);
  EXPECT_EQ(ch_tbbo2.flags, 129);
  EXPECT_EQ(ch_tbbo2.depth, 0);
  EXPECT_EQ(ch_tbbo2.ts_recv.time_since_epoch().count(), 1609160400108142648);
  EXPECT_EQ(ch_tbbo2.ts_in_delta.count(), 20728);
  EXPECT_EQ(ch_tbbo2.sequence, 1170414);
  EXPECT_EQ(ch_tbbo2.levels[0].bid_px, 3720250000000);
  EXPECT_EQ(ch_tbbo2.levels[0].ask_px, 3720500000000);
  EXPECT_EQ(ch_tbbo2.levels[0].bid_sz, 21);
  EXPECT_EQ(ch_tbbo2.levels[0].ask_sz, 22);
  EXPECT_EQ(ch_tbbo2.levels[0].bid_ct, 13);
  EXPECT_EQ(ch_tbbo2.levels[0].ask_ct, 15);
}

TEST_P(DbnDecoderSchemaTests, TestDecodeTrades) {
  const auto extension = GetParam().first;
  const auto version = GetParam().second;
  ReadFromFile("trades", extension, version);

  const Metadata ch_metadata = channel_target_->DecodeMetadata();
  const Metadata f_metadata = file_target_->DecodeMetadata();
  EXPECT_EQ(ch_metadata, f_metadata);
  EXPECT_EQ(ch_metadata.version, version);
  EXPECT_EQ(ch_metadata.dataset, dataset::kGlbxMdp3);
  EXPECT_EQ(ch_metadata.schema, Schema::Trades);
  EXPECT_EQ(ch_metadata.start.time_since_epoch().count(), 1609160400000000000);
  EXPECT_EQ(ch_metadata.end.time_since_epoch().count(), 1609200000000000000);
  EXPECT_EQ(ch_metadata.limit, 2);
  EXPECT_EQ(ch_metadata.stype_in, SType::RawSymbol);
  EXPECT_EQ(ch_metadata.stype_out, SType::InstrumentId);
  EXPECT_EQ(ch_metadata.symbols, std::vector<std::string>{"ESH1"});
  EXPECT_TRUE(ch_metadata.partial.empty());
  EXPECT_TRUE(ch_metadata.not_found.empty());
  AssertMappings(ch_metadata.mappings);

  const auto ch_record1 = channel_target_->DecodeRecord();
  const auto f_record1 = file_target_->DecodeRecord();
  ASSERT_NE(ch_record1, nullptr);
  ASSERT_NE(f_record1, nullptr);
  ASSERT_TRUE(ch_record1->Holds<TradeMsg>());
  ASSERT_TRUE(f_record1->Holds<TradeMsg>());
  const auto& ch_trade1 = ch_record1->Get<TradeMsg>();
  const auto& f_trade1 = f_record1->Get<TradeMsg>();
  EXPECT_EQ(ch_trade1, f_trade1);
  EXPECT_EQ(ch_trade1.hd.publisher_id, 1);
  EXPECT_EQ(ch_trade1.hd.instrument_id, 5482);
  EXPECT_EQ(ch_trade1.hd.ts_event.time_since_epoch().count(),
            1609160400098821953);
  EXPECT_EQ(ch_trade1.price, 3720250000000);
  EXPECT_EQ(ch_trade1.size, 5);
  EXPECT_EQ(ch_trade1.action, Action::Trade);
  EXPECT_EQ(ch_trade1.side, Side::Ask);
  EXPECT_EQ(ch_trade1.flags, 129);
  EXPECT_EQ(ch_trade1.depth, 0);
  EXPECT_EQ(ch_trade1.ts_recv.time_since_epoch().count(), 1609160400099150057);
  EXPECT_EQ(ch_trade1.ts_in_delta.count(), 19251);
  EXPECT_EQ(ch_trade1.sequence, 1170380);

  const auto ch_record2 = channel_target_->DecodeRecord();
  const auto f_record2 = file_target_->DecodeRecord();
  ASSERT_NE(ch_record2, nullptr);
  ASSERT_NE(f_record2, nullptr);
  ASSERT_TRUE(ch_record2->Holds<TradeMsg>());
  ASSERT_TRUE(f_record2->Holds<TradeMsg>());
  const auto& ch_trade2 = ch_record2->Get<TradeMsg>();
  const auto& f_trade2 = f_record2->Get<TradeMsg>();
  EXPECT_EQ(ch_trade2, f_trade2);
  EXPECT_EQ(ch_trade2.hd.publisher_id, 1);
  EXPECT_EQ(ch_trade2.hd.instrument_id, 5482);
  EXPECT_EQ(ch_trade2.hd.ts_event.time_since_epoch().count(),
            1609160400107665963);
  EXPECT_EQ(ch_trade2.price, 3720250000000);
  EXPECT_EQ(ch_trade2.size, 21);
  EXPECT_EQ(ch_trade2.action, Action::Trade);
  EXPECT_EQ(ch_trade2.side, Side::Ask);
  EXPECT_EQ(ch_trade2.flags, 129);
  EXPECT_EQ(ch_trade2.depth, 0);
  EXPECT_EQ(ch_trade2.ts_recv.time_since_epoch().count(), 1609160400108142648);
  EXPECT_EQ(ch_trade2.ts_in_delta.count(), 20728);
  EXPECT_EQ(ch_trade2.sequence, 1170414);
}

TEST_P(DbnDecoderSchemaTests, TestDecodeOhlcv1D) {
  const auto extension = GetParam().first;
  const auto version = GetParam().second;
  ReadFromFile("ohlcv-1d", extension, version);

  const Metadata ch_metadata = channel_target_->DecodeMetadata();
  const Metadata f_metadata = file_target_->DecodeMetadata();
  EXPECT_EQ(ch_metadata, f_metadata);
  EXPECT_EQ(ch_metadata.version, version);
  EXPECT_EQ(ch_metadata.dataset, dataset::kGlbxMdp3);
  EXPECT_EQ(ch_metadata.schema, Schema::Ohlcv1D);
  EXPECT_EQ(ch_metadata.start.time_since_epoch().count(), 1609160400000000000);
  EXPECT_EQ(ch_metadata.end.time_since_epoch().count(), 1609200000000000000);
  EXPECT_EQ(ch_metadata.limit, 2);
  EXPECT_EQ(ch_metadata.stype_in, SType::RawSymbol);
  EXPECT_EQ(ch_metadata.stype_out, SType::InstrumentId);
  EXPECT_EQ(ch_metadata.symbols, std::vector<std::string>{"ESH1"});
  EXPECT_TRUE(ch_metadata.partial.empty());
  EXPECT_TRUE(ch_metadata.not_found.empty());
  AssertMappings(ch_metadata.mappings);
}

TEST_P(DbnDecoderSchemaTests, TestDecodeOhlcv1H) {
  const auto extension = GetParam().first;
  const auto version = GetParam().second;
  ReadFromFile("ohlcv-1h", extension, version);

  const Metadata ch_metadata = channel_target_->DecodeMetadata();
  const Metadata f_metadata = file_target_->DecodeMetadata();
  EXPECT_EQ(ch_metadata, f_metadata);
  EXPECT_EQ(ch_metadata.version, version);
  EXPECT_EQ(ch_metadata.dataset, dataset::kGlbxMdp3);
  EXPECT_EQ(ch_metadata.schema, Schema::Ohlcv1H);
  EXPECT_EQ(ch_metadata.start.time_since_epoch().count(), 1609160400000000000);
  EXPECT_EQ(ch_metadata.end.time_since_epoch().count(), 1609200000000000000);
  EXPECT_EQ(ch_metadata.limit, 2);
  EXPECT_EQ(ch_metadata.stype_in, SType::RawSymbol);
  EXPECT_EQ(ch_metadata.stype_out, SType::InstrumentId);
  EXPECT_EQ(ch_metadata.symbols, std::vector<std::string>{"ESH1"});
  EXPECT_TRUE(ch_metadata.partial.empty());
  EXPECT_TRUE(ch_metadata.not_found.empty());
  AssertMappings(ch_metadata.mappings);

  const auto ch_record1 = channel_target_->DecodeRecord();
  const auto f_record1 = file_target_->DecodeRecord();
  ASSERT_NE(ch_record1, nullptr);
  ASSERT_NE(f_record1, nullptr);
  ASSERT_TRUE(ch_record1->Holds<OhlcvMsg>());
  ASSERT_TRUE(f_record1->Holds<OhlcvMsg>());
  const auto& ch_ohlcv1 = ch_record1->Get<OhlcvMsg>();
  const auto& f_ohlcv1 = f_record1->Get<OhlcvMsg>();
  EXPECT_EQ(ch_ohlcv1, f_ohlcv1);
  EXPECT_EQ(ch_ohlcv1.hd.publisher_id, 1);
  EXPECT_EQ(ch_ohlcv1.hd.instrument_id, 5482);
  EXPECT_EQ(ch_ohlcv1.hd.ts_event.time_since_epoch().count(),
            1609160400000000000);
  EXPECT_EQ(ch_ohlcv1.open, 372025000000000);
  EXPECT_EQ(ch_ohlcv1.high, 372350000000000);
  EXPECT_EQ(ch_ohlcv1.low, 372025000000000);
  EXPECT_EQ(ch_ohlcv1.close, 372225000000000);
  EXPECT_EQ(ch_ohlcv1.volume, 9385);

  const auto ch_record2 = channel_target_->DecodeRecord();
  const auto f_record2 = file_target_->DecodeRecord();
  ASSERT_NE(ch_record2, nullptr);
  ASSERT_NE(f_record2, nullptr);
  ASSERT_TRUE(ch_record2->Holds<OhlcvMsg>());
  ASSERT_TRUE(f_record2->Holds<OhlcvMsg>());
  const auto& ch_ohlcv2 = ch_record2->Get<OhlcvMsg>();
  const auto& f_ohlcv2 = f_record2->Get<OhlcvMsg>();
  EXPECT_EQ(ch_ohlcv2, f_ohlcv2);
  EXPECT_EQ(ch_ohlcv2.hd.publisher_id, 1);
  EXPECT_EQ(ch_ohlcv2.hd.instrument_id, 5482);
  EXPECT_EQ(ch_ohlcv2.hd.ts_event.time_since_epoch().count(),
            1609164000000000000);
  EXPECT_EQ(ch_ohlcv2.open, 372225000000000);
  EXPECT_EQ(ch_ohlcv2.high, 372450000000000);
  EXPECT_EQ(ch_ohlcv2.low, 371600000000000);
  EXPECT_EQ(ch_ohlcv2.close, 371950000000000);
  EXPECT_EQ(ch_ohlcv2.volume, 112698);
}

TEST_P(DbnDecoderSchemaTests, TestDecodeOhlcv1M) {
  const auto extension = GetParam().first;
  const auto version = GetParam().second;
  ReadFromFile("ohlcv-1m", extension, version);

  const Metadata ch_metadata = channel_target_->DecodeMetadata();
  const Metadata f_metadata = file_target_->DecodeMetadata();
  EXPECT_EQ(ch_metadata, f_metadata);
  EXPECT_EQ(ch_metadata.version, version);
  EXPECT_EQ(ch_metadata.dataset, dataset::kGlbxMdp3);
  EXPECT_EQ(ch_metadata.schema, Schema::Ohlcv1M);
  EXPECT_EQ(ch_metadata.start.time_since_epoch().count(), 1609160400000000000);
  EXPECT_EQ(ch_metadata.end.time_since_epoch().count(), 1609200000000000000);
  EXPECT_EQ(ch_metadata.limit, 2);
  EXPECT_EQ(ch_metadata.stype_in, SType::RawSymbol);
  EXPECT_EQ(ch_metadata.stype_out, SType::InstrumentId);
  EXPECT_EQ(ch_metadata.symbols, std::vector<std::string>{"ESH1"});
  EXPECT_TRUE(ch_metadata.partial.empty());
  EXPECT_TRUE(ch_metadata.not_found.empty());
  AssertMappings(ch_metadata.mappings);

  const auto ch_record1 = channel_target_->DecodeRecord();
  const auto f_record1 = file_target_->DecodeRecord();
  ASSERT_NE(ch_record1, nullptr);
  ASSERT_NE(f_record1, nullptr);
  ASSERT_TRUE(ch_record1->Holds<OhlcvMsg>());
  ASSERT_TRUE(f_record1->Holds<OhlcvMsg>());
  const auto& ch_ohlcv1 = ch_record1->Get<OhlcvMsg>();
  const auto& f_ohlcv1 = f_record1->Get<OhlcvMsg>();
  EXPECT_EQ(ch_ohlcv1, f_ohlcv1);
  EXPECT_EQ(ch_ohlcv1.hd.publisher_id, 1);
  EXPECT_EQ(ch_ohlcv1.hd.instrument_id, 5482);
  EXPECT_EQ(ch_ohlcv1.hd.ts_event.time_since_epoch().count(),
            1609160400000000000);
  EXPECT_EQ(ch_ohlcv1.open, 372025000000000);
  EXPECT_EQ(ch_ohlcv1.high, 372150000000000);
  EXPECT_EQ(ch_ohlcv1.low, 372025000000000);
  EXPECT_EQ(ch_ohlcv1.close, 372100000000000);
  EXPECT_EQ(ch_ohlcv1.volume, 353);

  const auto ch_record2 = channel_target_->DecodeRecord();
  const auto f_record2 = file_target_->DecodeRecord();
  ASSERT_NE(ch_record2, nullptr);
  ASSERT_NE(f_record2, nullptr);
  ASSERT_TRUE(ch_record2->Holds<OhlcvMsg>());
  ASSERT_TRUE(f_record2->Holds<OhlcvMsg>());
  const auto& ch_ohlcv2 = ch_record2->Get<OhlcvMsg>();
  const auto& f_ohlcv2 = f_record2->Get<OhlcvMsg>();
  EXPECT_EQ(ch_ohlcv2, f_ohlcv2);
  EXPECT_EQ(ch_ohlcv2.hd.publisher_id, 1);
  EXPECT_EQ(ch_ohlcv2.hd.instrument_id, 5482);
  EXPECT_EQ(ch_ohlcv2.hd.ts_event.time_since_epoch().count(),
            1609160460000000000);
  EXPECT_EQ(ch_ohlcv2.open, 372100000000000);
  EXPECT_EQ(ch_ohlcv2.high, 372150000000000);
  EXPECT_EQ(ch_ohlcv2.low, 372100000000000);
  EXPECT_EQ(ch_ohlcv2.close, 372150000000000);
  EXPECT_EQ(ch_ohlcv2.volume, 152);
}

TEST_P(DbnDecoderSchemaTests, TestDecodeOhlcv1S) {
  const auto extension = GetParam().first;
  const auto version = GetParam().second;
  ReadFromFile("ohlcv-1s", extension, version);

  const Metadata ch_metadata = channel_target_->DecodeMetadata();
  const Metadata f_metadata = file_target_->DecodeMetadata();
  EXPECT_EQ(ch_metadata, f_metadata);
  EXPECT_EQ(ch_metadata.version, version);
  EXPECT_EQ(ch_metadata.dataset, dataset::kGlbxMdp3);
  EXPECT_EQ(ch_metadata.schema, Schema::Ohlcv1S);
  EXPECT_EQ(ch_metadata.start.time_since_epoch().count(), 1609160400000000000);
  EXPECT_EQ(ch_metadata.end.time_since_epoch().count(), 1609200000000000000);
  EXPECT_EQ(ch_metadata.limit, 2);
  EXPECT_EQ(ch_metadata.stype_in, SType::RawSymbol);
  EXPECT_EQ(ch_metadata.stype_out, SType::InstrumentId);
  EXPECT_EQ(ch_metadata.symbols, std::vector<std::string>{"ESH1"});
  EXPECT_TRUE(ch_metadata.partial.empty());
  EXPECT_TRUE(ch_metadata.not_found.empty());
  AssertMappings(ch_metadata.mappings);

  const auto ch_record1 = channel_target_->DecodeRecord();
  const auto f_record1 = file_target_->DecodeRecord();
  ASSERT_NE(ch_record1, nullptr);
  ASSERT_NE(f_record1, nullptr);
  ASSERT_TRUE(ch_record1->Holds<OhlcvMsg>());
  ASSERT_TRUE(f_record1->Holds<OhlcvMsg>());
  const auto& ch_ohlcv1 = ch_record1->Get<OhlcvMsg>();
  const auto& f_ohlcv1 = f_record1->Get<OhlcvMsg>();
  EXPECT_EQ(ch_ohlcv1, f_ohlcv1);
  EXPECT_EQ(ch_ohlcv1.hd.publisher_id, 1);
  EXPECT_EQ(ch_ohlcv1.hd.instrument_id, 5482);
  EXPECT_EQ(ch_ohlcv1.hd.ts_event.time_since_epoch().count(),
            1609160400000000000);
  EXPECT_EQ(ch_ohlcv1.open, 372025000000000);
  EXPECT_EQ(ch_ohlcv1.high, 372050000000000);
  EXPECT_EQ(ch_ohlcv1.low, 372025000000000);
  EXPECT_EQ(ch_ohlcv1.close, 372050000000000);
  EXPECT_EQ(ch_ohlcv1.volume, 57);

  const auto ch_record2 = channel_target_->DecodeRecord();
  const auto f_record2 = file_target_->DecodeRecord();
  ASSERT_NE(ch_record2, nullptr);
  ASSERT_NE(f_record2, nullptr);
  ASSERT_TRUE(ch_record2->Holds<OhlcvMsg>());
  ASSERT_TRUE(f_record2->Holds<OhlcvMsg>());
  const auto& ch_ohlcv2 = ch_record2->Get<OhlcvMsg>();
  const auto& f_ohlcv2 = f_record2->Get<OhlcvMsg>();
  EXPECT_EQ(ch_ohlcv2, f_ohlcv2);
  EXPECT_EQ(ch_ohlcv2.hd.publisher_id, 1);
  EXPECT_EQ(ch_ohlcv2.hd.instrument_id, 5482);
  EXPECT_EQ(ch_ohlcv2.hd.ts_event.time_since_epoch().count(),
            1609160401000000000);
  EXPECT_EQ(ch_ohlcv2.open, 372050000000000);
  EXPECT_EQ(ch_ohlcv2.high, 372050000000000);
  EXPECT_EQ(ch_ohlcv2.low, 372050000000000);
  EXPECT_EQ(ch_ohlcv2.close, 372050000000000);
  EXPECT_EQ(ch_ohlcv2.volume, 13);
}

TEST_P(DbnDecoderSchemaTests, TestDecodeDefinition) {
  const auto extension = GetParam().first;
  const auto version = GetParam().second;
  ReadFromFile("definition", extension, version);

  const Metadata ch_metadata = channel_target_->DecodeMetadata();
  const Metadata f_metadata = file_target_->DecodeMetadata();
  EXPECT_EQ(ch_metadata, f_metadata);
  EXPECT_EQ(ch_metadata.version, version);
  EXPECT_EQ(ch_metadata.dataset, dataset::kXnasItch);
  EXPECT_EQ(ch_metadata.schema, Schema::Definition);
  EXPECT_EQ(ch_metadata.start.time_since_epoch().count(), 1633305600000000000);
  EXPECT_EQ(ch_metadata.end.time_since_epoch().count(), 1641254400000000000);
  EXPECT_EQ(ch_metadata.limit, 2);
  EXPECT_EQ(ch_metadata.stype_in, SType::RawSymbol);
  EXPECT_EQ(ch_metadata.stype_out, SType::InstrumentId);
  EXPECT_EQ(ch_metadata.symbols, std::vector<std::string>{"MSFT"});
  EXPECT_TRUE(ch_metadata.partial.empty());
  EXPECT_TRUE(ch_metadata.not_found.empty());
  EXPECT_EQ(ch_metadata.mappings.size(), 1);
  const auto& mapping = ch_metadata.mappings.at(0);
  EXPECT_EQ(mapping.raw_symbol, "MSFT");
  ASSERT_EQ(mapping.intervals.size(), 62);
  const auto& interval = mapping.intervals.at(0);
  EXPECT_EQ(interval.symbol, "6819");
  EXPECT_EQ(interval.start_date, 20211004);
  EXPECT_EQ(interval.end_date, 20211005);

  const auto ch_record1 = channel_target_->DecodeRecord();
  const auto f_record1 = file_target_->DecodeRecord();
  ASSERT_NE(ch_record1, nullptr);
  ASSERT_NE(f_record1, nullptr);

  const auto ch_record2 = channel_target_->DecodeRecord();
  const auto f_record2 = file_target_->DecodeRecord();
  ASSERT_NE(ch_record2, nullptr);
  ASSERT_NE(f_record2, nullptr);
  if (version == 1) {
    AssertDefEq<InstrumentDefMsgV1>(ch_record1, f_record1);
    AssertDefEq<InstrumentDefMsgV1>(ch_record2, f_record2);
  } else {
    AssertDefEq<InstrumentDefMsgV2>(ch_record1, f_record1);
    AssertDefEq<InstrumentDefMsgV2>(ch_record2, f_record2);
  }
}

TEST_P(DbnDecoderSchemaTests, TestDecodeImbalance) {
  const auto extension = GetParam().first;
  const auto version = GetParam().second;
  ReadFromFile("imbalance", extension, version);

  const Metadata ch_metadata = channel_target_->DecodeMetadata();
  const Metadata f_metadata = file_target_->DecodeMetadata();
  EXPECT_EQ(ch_metadata, f_metadata);
  EXPECT_EQ(ch_metadata.version, version);
  EXPECT_EQ(ch_metadata.dataset, dataset::kXnasItch);
  EXPECT_EQ(ch_metadata.schema, Schema::Imbalance);
  EXPECT_EQ(ch_metadata.start.time_since_epoch().count(), 1633305600000000000);
  EXPECT_EQ(ch_metadata.end.time_since_epoch().count(), 1641254400000000000);
  EXPECT_EQ(ch_metadata.limit, 2);
  EXPECT_EQ(ch_metadata.stype_in, SType::RawSymbol);
  EXPECT_EQ(ch_metadata.stype_out, SType::InstrumentId);
  EXPECT_EQ(ch_metadata.symbols, std::vector<std::string>{"SPOT"});
  EXPECT_TRUE(ch_metadata.partial.empty());
  EXPECT_TRUE(ch_metadata.not_found.empty());
  EXPECT_EQ(ch_metadata.mappings.size(), 1);

  const auto ch_record1 = channel_target_->DecodeRecord();
  const auto f_record1 = file_target_->DecodeRecord();
  ASSERT_NE(ch_record1, nullptr);
  ASSERT_NE(f_record1, nullptr);
  ASSERT_TRUE(ch_record1->Holds<ImbalanceMsg>());
  ASSERT_TRUE(f_record1->Holds<ImbalanceMsg>());
  const auto& ch_imbalance1 = ch_record1->Get<ImbalanceMsg>();
  const auto& f_imbalance1 = f_record1->Get<ImbalanceMsg>();
  EXPECT_EQ(ch_imbalance1, f_imbalance1);
  EXPECT_EQ(ch_imbalance1.ref_price, 229430000000);

  const auto ch_record2 = channel_target_->DecodeRecord();
  const auto f_record2 = file_target_->DecodeRecord();
  ASSERT_NE(ch_record2, nullptr);
  ASSERT_NE(f_record2, nullptr);
  ASSERT_TRUE(ch_record2->Holds<ImbalanceMsg>());
  ASSERT_TRUE(f_record2->Holds<ImbalanceMsg>());
  const auto& ch_imbalance2 = ch_record2->Get<ImbalanceMsg>();
  const auto& f_imbalance2 = f_record2->Get<ImbalanceMsg>();
  EXPECT_EQ(ch_imbalance2, f_imbalance2);
  EXPECT_EQ(ch_imbalance2.ref_price, 229990000000);
}

TEST_P(DbnDecoderSchemaTests, TestDecodeStatistics) {
  const auto extension = GetParam().first;
  const auto version = GetParam().second;
  ReadFromFile("statistics", extension, version);

  const Metadata ch_metadata = channel_target_->DecodeMetadata();
  const Metadata f_metadata = file_target_->DecodeMetadata();
  EXPECT_EQ(ch_metadata, f_metadata);
  EXPECT_EQ(ch_metadata.version, version);
  EXPECT_EQ(ch_metadata.dataset, dataset::kGlbxMdp3);
  EXPECT_EQ(ch_metadata.schema, Schema::Statistics);
  EXPECT_EQ(ch_metadata.start.time_since_epoch().count(), 2814749767106560);
  EXPECT_EQ(ch_metadata.end.time_since_epoch().count(), 18446744073709551615UL);
  EXPECT_EQ(ch_metadata.limit, 2);
  EXPECT_EQ(ch_metadata.stype_in, SType::InstrumentId);
  EXPECT_EQ(ch_metadata.stype_out, SType::InstrumentId);
  EXPECT_TRUE(ch_metadata.symbols.empty());
  EXPECT_TRUE(ch_metadata.partial.empty());
  EXPECT_TRUE(ch_metadata.not_found.empty());
  EXPECT_TRUE(ch_metadata.mappings.empty());

  const auto ch_record1 = channel_target_->DecodeRecord();
  const auto f_record1 = file_target_->DecodeRecord();
  ASSERT_NE(ch_record1, nullptr);
  ASSERT_NE(f_record1, nullptr);
  ASSERT_TRUE(ch_record1->Holds<StatMsg>());
  ASSERT_TRUE(f_record1->Holds<StatMsg>());
  const auto& ch_stat1 = ch_record1->Get<StatMsg>();
  const auto& f_stat1 = f_record1->Get<StatMsg>();
  EXPECT_EQ(ch_stat1, f_stat1);
  EXPECT_EQ(ch_stat1.stat_type, StatType::LowestOffer);
  EXPECT_EQ(ch_stat1.price, 100 * kFixedPriceScale);

  const auto ch_record2 = channel_target_->DecodeRecord();
  const auto f_record2 = file_target_->DecodeRecord();
  ASSERT_NE(ch_record2, nullptr);
  ASSERT_NE(f_record2, nullptr);
  ASSERT_TRUE(ch_record2->Holds<StatMsg>());
  ASSERT_TRUE(f_record2->Holds<StatMsg>());
  const auto& ch_stat2 = ch_record2->Get<StatMsg>();
  const auto& f_stat2 = f_record2->Get<StatMsg>();
  EXPECT_EQ(ch_stat2, f_stat2);
  EXPECT_EQ(ch_stat2.stat_type, StatType::TradingSessionHighPrice);
  EXPECT_EQ(ch_stat2.price, 100 * kFixedPriceScale);
}
}  // namespace test
}  // namespace databento
