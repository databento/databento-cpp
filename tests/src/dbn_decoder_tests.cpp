#include <date/date.h>
#include <gtest/gtest.h>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>  // ifstream
#include <ios>      // streamsize, ios::binary, ios::ate
#include <limits>
#include <memory>
#include <tuple>
#include <vector>

#include "databento/compat.hpp"
#include "databento/constants.hpp"
#include "databento/datetime.hpp"
#include "databento/dbn.hpp"
#include "databento/dbn_decoder.hpp"
#include "databento/dbn_encoder.hpp"
#include "databento/detail/buffer.hpp"
#include "databento/detail/scoped_thread.hpp"
#include "databento/detail/zstd_stream.hpp"
#include "databento/enums.hpp"
#include "databento/exceptions.hpp"
#include "databento/file_stream.hpp"
#include "databento/iwritable.hpp"
#include "databento/log.hpp"
#include "databento/record.hpp"
#include "databento/v1.hpp"
#include "databento/v2.hpp"
#include "databento/v3.hpp"
#include "databento/with_ts_out.hpp"
#include "mock/mock_log_receiver.hpp"

namespace databento::tests {
class DbnDecoderTests : public testing::Test {
 public:
  std::unique_ptr<DbnDecoder> target_;
  detail::ScopedThread write_thread_;
  mock::MockLogReceiver logger_ =
      mock::MockLogReceiver::AssertNoLogs(LogLevel::Warning);

  void ReadFromFile(const std::string& schema_str, const std::string& extension,
                    std::uint8_t version) {
    ReadFromFile(schema_str, extension, version, VersionUpgradePolicy::AsIs);
  }

  void ReadFromFile(const std::string& schema_str, const std::string& extension,
                    std::uint8_t version, VersionUpgradePolicy upgrade_policy) {
    const std::string file_path =
        TEST_DATA_DIR "/test_data." + schema_str +
        (version == 0 ? "" : ".v" + std::to_string(+version)) + extension;
    // Channel setup
    write_thread_ = detail::ScopedThread{[this, file_path] {
      std::ifstream input_file{file_path, std::ios::binary | std::ios::ate};
      ASSERT_TRUE(input_file.good()) << "Failed to open: " << file_path;
      const auto size = static_cast<std::size_t>(input_file.tellg());
      input_file.seekg(0, std::ios::beg);
      std::vector<char> buffer(size);
      input_file.read(buffer.data(), static_cast<std::streamsize>(size));
      ASSERT_EQ(input_file.gcount(), size);
    }};
    // File setup
    target_ = std::make_unique<DbnDecoder>(
        &logger_, std::make_unique<InFileStream>(file_path), upgrade_policy);
  }

  static void AssertMappings(const std::vector<SymbolMapping>& mappings) {
    ASSERT_EQ(mappings.size(), 1);
    const auto& mapping = mappings.at(0);
    EXPECT_EQ(mapping.raw_symbol, "ESH1");
    ASSERT_EQ(mapping.intervals.size(), 1);
    const auto& interval = mapping.intervals.at(0);
    EXPECT_EQ(interval.symbol, "5482");
    EXPECT_EQ(interval.start_date, date::year{2020} / 12 / 28);
    EXPECT_EQ(interval.end_date, date::year{2020} / 12 / 29);
  }
};

template <typename D>
void AssertDefHas(const Record* record) {
  ASSERT_TRUE(record->Holds<D>());
  const auto& def = record->Get<D>();
  EXPECT_STREQ(def.Exchange(), "XNAS");
  EXPECT_EQ(def.security_update_action, SecurityUpdateAction::Add);
  EXPECT_EQ(def.min_lot_size_round_lot, 100);
  EXPECT_EQ(def.instrument_class, InstrumentClass::Stock);
  EXPECT_EQ(def.strike_price, kUndefPrice);
}

template <typename S, typename Q>
void AssertStatHas(const Record* record, StatType stat_type, std::int64_t price,
                   Q quantity) {
  ASSERT_TRUE(record->Holds<S>());
  const auto& stat = record->Get<S>();
  ASSERT_EQ(stat.stat_type, stat_type);
  ASSERT_EQ(stat.price, price);
  ASSERT_EQ(stat.quantity, quantity);
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
  ReadFromFile("definition", ".dbn.zst", 1, VersionUpgradePolicy::UpgradeToV3);

  const Metadata metadata = target_->DecodeMetadata();
  EXPECT_EQ(metadata, metadata);
  EXPECT_EQ(metadata.version, kDbnVersion);
  EXPECT_EQ(metadata.dataset, dataset::kXnasItch);
  EXPECT_EQ(metadata.schema, Schema::Definition);
  EXPECT_EQ(metadata.start.time_since_epoch().count(), 1633305600000000000);
  EXPECT_EQ(metadata.end.time_since_epoch().count(), 1641254400000000000);
  EXPECT_EQ(metadata.limit, 2);
  EXPECT_EQ(metadata.stype_in, SType::RawSymbol);
  EXPECT_EQ(metadata.stype_out, SType::InstrumentId);
  EXPECT_EQ(metadata.symbols, std::vector<std::string>{"MSFT"});
  EXPECT_TRUE(metadata.partial.empty());
  EXPECT_TRUE(metadata.not_found.empty());
  EXPECT_EQ(metadata.mappings.size(), 1);
  const auto& mapping = metadata.mappings.at(0);
  EXPECT_EQ(mapping.raw_symbol, "MSFT");
  ASSERT_EQ(mapping.intervals.size(), 62);
  const auto& interval = mapping.intervals.at(0);
  EXPECT_EQ(interval.symbol, "6819");
  EXPECT_EQ(interval.start_date, date::year{2021} / 10 / 4);
  EXPECT_EQ(interval.end_date, date::year{2021} / 10 / 5);

  const auto record1 = target_->DecodeRecord();
  ASSERT_NE(record1, nullptr);
  record1->Get<v3::InstrumentDefMsg>();

  const auto record2 = target_->DecodeRecord();
  ASSERT_NE(record2, nullptr);
  record2->Get<v3::InstrumentDefMsg>();
}

TEST_F(DbnDecoderTests, TestDecodeStatUpgrade) {
  ReadFromFile("statistics", ".dbn.zst", 2, VersionUpgradePolicy::UpgradeToV3);
  const Metadata metadata = target_->DecodeMetadata();
  EXPECT_EQ(metadata, metadata);
  EXPECT_EQ(metadata.version, kDbnVersion);

  const auto record1 = target_->DecodeRecord();
  ASSERT_NE(record1, nullptr);
  AssertStatHas<v3::StatMsg>(record1, StatType::LowestOffer, 100 * kFixedPriceScale,
                             v3::kUndefStatQuantity);

  const auto record2 = target_->DecodeRecord();
  ASSERT_NE(record2, nullptr);
  AssertStatHas<v3::StatMsg>(record2, StatType::TradingSessionHighPrice,
                             100 * kFixedPriceScale, v3::kUndefStatQuantity);
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
  WithTsOut<SymbolMappingMsgV1> orig{sym_map,
                                     UnixNanos{std::chrono::system_clock::now()}};
  std::array<std::byte, kMaxRecordLen> compat_buffer{};
  const auto res = DbnDecoder::DecodeRecordCompat(
      1, VersionUpgradePolicy::UpgradeToV2, true, &compat_buffer, Record{&orig.rec.hd});
  const auto& upgraded = res.Get<WithTsOut<SymbolMappingMsgV2>>();
  EXPECT_EQ(orig.rec.hd.rtype, upgraded.rec.hd.rtype);
  EXPECT_EQ(orig.rec.hd.instrument_id, upgraded.rec.hd.instrument_id);
  EXPECT_EQ(orig.rec.hd.publisher_id, upgraded.rec.hd.publisher_id);
  EXPECT_EQ(orig.rec.hd.ts_event, upgraded.rec.hd.ts_event);
  EXPECT_EQ(orig.ts_out, upgraded.ts_out);
  EXPECT_STREQ(orig.rec.STypeInSymbol(), upgraded.rec.STypeInSymbol());
  EXPECT_STREQ(orig.rec.STypeOutSymbol(), upgraded.rec.STypeOutSymbol());
  EXPECT_EQ(static_cast<std::uint8_t>(upgraded.rec.stype_in),
            std::numeric_limits<std::uint8_t>::max());
  EXPECT_EQ(static_cast<std::uint8_t>(upgraded.rec.stype_out),
            std::numeric_limits<std::uint8_t>::max());
  // `length` properly set
  EXPECT_EQ(upgraded.rec.hd.Size(), sizeof(upgraded));
  // used compat buffer
  EXPECT_EQ(reinterpret_cast<const std::byte*>(&upgraded), compat_buffer.data());
}

TEST_F(DbnDecoderTests, TestUpgradeMbp1WithTsOut) {
  WithTsOut<Mbp1Msg> orig{
      Mbp1Msg{
          {sizeof(Mbp1Msg) / RecordHeader::kLengthMultiplier, RType::Mbp1, {}, {}, {}},
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
  std::array<std::byte, kMaxRecordLen> compat_buffer{};
  const auto res = DbnDecoder::DecodeRecordCompat(
      1, VersionUpgradePolicy::UpgradeToV2, true, &compat_buffer, Record{&orig.rec.hd});
  const auto& upgraded = res.Get<WithTsOut<Mbp1Msg>>();
  // compat buffer unused and pointer unchanged
  ASSERT_EQ(&orig, &upgraded);
}

class DbnDecoderSchemaTests
    : public DbnDecoderTests,
      public testing::WithParamInterface<std::pair<const char*, std::uint8_t>> {};

INSTANTIATE_TEST_SUITE_P(
    TestFiles, DbnDecoderSchemaTests,
    testing::Values(std::make_pair(".dbn.zst", 1), std::make_pair(".dbn.zst", 2),
                    std::make_pair(".dbn.zst", 3)),
    [](const testing::TestParamInfo<std::pair<const char*, std::uint8_t>>& test_info) {
      const auto extension = test_info.param.first;
      const auto version = test_info.param.second;
      const auto size = ::strlen(extension);
      return "ZstdDBNv" + std::to_string(version);
    });

// Expected data for these tests obtained using the `dbn` CLI tool

TEST_P(DbnDecoderSchemaTests, TestDecodeMbo) {
  const auto [extension, version] = GetParam();
  ReadFromFile("mbo", extension, version);

  const Metadata metadata = target_->DecodeMetadata();
  EXPECT_EQ(metadata, metadata);
  EXPECT_EQ(metadata.version, version);
  EXPECT_EQ(metadata.dataset, dataset::kGlbxMdp3);
  EXPECT_EQ(metadata.schema, Schema::Mbo);
  EXPECT_EQ(metadata.start.time_since_epoch().count(), 1609160400000000000);
  EXPECT_EQ(metadata.end.time_since_epoch().count(), 1609200000000000000);
  EXPECT_EQ(metadata.limit, 2);
  EXPECT_EQ(metadata.stype_in, SType::RawSymbol);
  EXPECT_EQ(metadata.stype_out, SType::InstrumentId);
  EXPECT_EQ(metadata.symbols, std::vector<std::string>{"ESH1"});
  EXPECT_TRUE(metadata.partial.empty());
  EXPECT_TRUE(metadata.not_found.empty());

  const auto record1 = target_->DecodeRecord();
  ASSERT_NE(record1, nullptr);
  ASSERT_TRUE(record1->Holds<MboMsg>());
  const auto& mbo1 = record1->Get<MboMsg>();
  EXPECT_EQ(mbo1, mbo1);
  EXPECT_EQ(mbo1.hd.publisher_id, 1);
  EXPECT_EQ(mbo1.hd.instrument_id, 5482);
  EXPECT_EQ(mbo1.hd.ts_event.time_since_epoch().count(), 1609160400000429831);
  EXPECT_EQ(mbo1.order_id, 647784973705);
  EXPECT_EQ(mbo1.price, 3722750000000);
  EXPECT_EQ(mbo1.size, 1);
  EXPECT_EQ(mbo1.flags.Raw(), 128);
  EXPECT_EQ(mbo1.channel_id, 0);
  EXPECT_EQ(mbo1.action, Action::Cancel);
  EXPECT_EQ(mbo1.side, Side::Ask);
  EXPECT_EQ(mbo1.ts_recv.time_since_epoch().count(), 1609160400000704060);
  EXPECT_EQ(mbo1.ts_in_delta.count(), 22993);
  EXPECT_EQ(mbo1.sequence, 1170352);

  const auto record2 = target_->DecodeRecord();
  ASSERT_NE(record2, nullptr);
  ASSERT_TRUE(record2->Holds<MboMsg>());
  const auto& mbo2 = record2->Get<MboMsg>();
  EXPECT_EQ(mbo2, mbo2);
  EXPECT_EQ(mbo2.hd.publisher_id, 1);
  EXPECT_EQ(mbo2.hd.instrument_id, 5482);
  EXPECT_EQ(mbo2.hd.ts_event.time_since_epoch().count(), 1609160400000431665);
  EXPECT_EQ(mbo2.order_id, 647784973631);
  EXPECT_EQ(mbo2.price, 3723000000000);
  EXPECT_EQ(mbo2.size, 1);
  EXPECT_EQ(mbo2.flags.Raw(), 128);
  EXPECT_EQ(mbo2.channel_id, 0);
  EXPECT_EQ(mbo2.action, Action::Cancel);
  EXPECT_EQ(mbo2.side, Side::Ask);
  EXPECT_EQ(mbo2.ts_recv.time_since_epoch().count(), 1609160400000711344);
  EXPECT_EQ(mbo2.ts_in_delta.count(), 19621);
  EXPECT_EQ(mbo2.sequence, 1170353);
}

TEST_P(DbnDecoderSchemaTests, TestDecodeMbp1) {
  const auto [extension, version] = GetParam();
  ReadFromFile("mbp-1", extension, version);

  const Metadata metadata = target_->DecodeMetadata();
  EXPECT_EQ(metadata, metadata);
  EXPECT_EQ(metadata.version, version);
  EXPECT_EQ(metadata.dataset, dataset::kGlbxMdp3);
  EXPECT_EQ(metadata.schema, Schema::Mbp1);
  EXPECT_EQ(metadata.start.time_since_epoch().count(), 1609160400000000000);
  EXPECT_EQ(metadata.end.time_since_epoch().count(), 1609200000000000000);
  EXPECT_EQ(metadata.limit, 2);
  EXPECT_EQ(metadata.stype_in, SType::RawSymbol);
  EXPECT_EQ(metadata.stype_out, SType::InstrumentId);
  EXPECT_EQ(metadata.symbols, std::vector<std::string>{"ESH1"});
  EXPECT_TRUE(metadata.partial.empty());
  EXPECT_TRUE(metadata.not_found.empty());
  AssertMappings(metadata.mappings);

  const auto record1 = target_->DecodeRecord();
  ASSERT_NE(record1, nullptr);
  ASSERT_TRUE(record1->Holds<Mbp1Msg>());
  const auto& mbp1 = record1->Get<Mbp1Msg>();
  EXPECT_EQ(mbp1.hd.publisher_id, 1);
  EXPECT_EQ(mbp1.hd.instrument_id, 5482);
  EXPECT_EQ(mbp1.hd.ts_event.time_since_epoch().count(), 1609160400006001487);
  EXPECT_EQ(mbp1.price, 3720500000000);
  EXPECT_EQ(mbp1.size, 1);
  EXPECT_EQ(mbp1.action, Action::Add);
  EXPECT_EQ(mbp1.side, Side::Ask);
  EXPECT_EQ(mbp1.flags.Raw(), 128);
  EXPECT_EQ(mbp1.depth, 0);
  EXPECT_EQ(mbp1.ts_recv.time_since_epoch().count(), 1609160400006136329);
  EXPECT_EQ(mbp1.ts_in_delta.count(), 17214);
  EXPECT_EQ(mbp1.sequence, 1170362);
  EXPECT_EQ(mbp1.levels[0].bid_px, 3720250000000);
  EXPECT_EQ(mbp1.levels[0].ask_px, 3720500000000);
  EXPECT_EQ(mbp1.levels[0].bid_sz, 24);
  EXPECT_EQ(mbp1.levels[0].ask_sz, 11);
  EXPECT_EQ(mbp1.levels[0].bid_ct, 15);
  EXPECT_EQ(mbp1.levels[0].ask_ct, 9);

  const auto record2 = target_->DecodeRecord();
  ASSERT_NE(record2, nullptr);
  ASSERT_TRUE(record2->Holds<Mbp1Msg>());
  const auto& mbp2 = record2->Get<Mbp1Msg>();
  EXPECT_EQ(mbp2, mbp2);
  EXPECT_EQ(mbp2.hd.publisher_id, 1);
  EXPECT_EQ(mbp2.hd.instrument_id, 5482);
  EXPECT_EQ(mbp2.hd.ts_event.time_since_epoch().count(), 1609160400006146661);
  EXPECT_EQ(mbp2.price, 3720500000000);
  EXPECT_EQ(mbp2.size, 1);
  EXPECT_EQ(mbp2.action, Action::Add);
  EXPECT_EQ(mbp2.side, Side::Ask);
  EXPECT_EQ(mbp2.flags.Raw(), 128);
  EXPECT_EQ(mbp2.depth, 0);
  EXPECT_EQ(mbp2.ts_recv.time_since_epoch().count(), 1609160400006246513);
  EXPECT_EQ(mbp2.ts_in_delta.count(), 18858);
  EXPECT_EQ(mbp2.sequence, 1170364);
  EXPECT_EQ(mbp2.levels[0].bid_px, 3720250000000);
  EXPECT_EQ(mbp2.levels[0].ask_px, 3720500000000);
  EXPECT_EQ(mbp2.levels[0].bid_sz, 24);
  EXPECT_EQ(mbp2.levels[0].ask_sz, 12);
  EXPECT_EQ(mbp2.levels[0].bid_ct, 15);
  EXPECT_EQ(mbp2.levels[0].ask_ct, 10);
}

TEST_P(DbnDecoderSchemaTests, TestDecodeMbp10) {
  const auto [extension, version] = GetParam();
  ReadFromFile("mbp-10", extension, version);

  const Metadata metadata = target_->DecodeMetadata();
  EXPECT_EQ(metadata.version, version);
  EXPECT_EQ(metadata.dataset, dataset::kGlbxMdp3);
  EXPECT_EQ(metadata.schema, Schema::Mbp10);
  EXPECT_EQ(metadata.start.time_since_epoch().count(), 1609160400000000000);
  EXPECT_EQ(metadata.end.time_since_epoch().count(), 1609200000000000000);
  EXPECT_EQ(metadata.limit, 2);
  EXPECT_EQ(metadata.stype_in, SType::RawSymbol);
  EXPECT_EQ(metadata.stype_out, SType::InstrumentId);
  EXPECT_EQ(metadata.symbols, std::vector<std::string>{"ESH1"});
  EXPECT_TRUE(metadata.partial.empty());
  EXPECT_TRUE(metadata.not_found.empty());
  AssertMappings(metadata.mappings);

  const auto record1 = target_->DecodeRecord();
  ASSERT_NE(record1, nullptr);
  ASSERT_TRUE(record1->Holds<Mbp10Msg>());
  const auto& mbp1 = record1->Get<Mbp10Msg>();
  EXPECT_EQ(mbp1.hd.publisher_id, 1);
  EXPECT_EQ(mbp1.hd.instrument_id, 5482);
  EXPECT_EQ(mbp1.hd.ts_event.time_since_epoch().count(), 1609160400000429831);
  EXPECT_EQ(mbp1.price, 3722750000000);
  EXPECT_EQ(mbp1.size, 1);
  EXPECT_EQ(mbp1.action, Action::Cancel);
  EXPECT_EQ(mbp1.side, Side::Ask);
  EXPECT_EQ(mbp1.flags.Raw(), 128);
  EXPECT_EQ(mbp1.depth, 9);
  EXPECT_EQ(mbp1.ts_recv.time_since_epoch().count(), 1609160400000704060);
  EXPECT_EQ(mbp1.ts_in_delta.count(), 22993);
  EXPECT_EQ(mbp1.sequence, 1170352);
  EXPECT_EQ(mbp1.levels[0].bid_px, 3720250000000);
  EXPECT_EQ(mbp1.levels[0].ask_px, 3720500000000);
  EXPECT_EQ(mbp1.levels[0].bid_sz, 24);
  EXPECT_EQ(mbp1.levels[0].ask_sz, 10);
  EXPECT_EQ(mbp1.levels[0].bid_ct, 15);
  EXPECT_EQ(mbp1.levels[0].ask_ct, 8);
  EXPECT_EQ(mbp1.levels[1].bid_px, 3720000000000);
  EXPECT_EQ(mbp1.levels[1].ask_px, 3720750000000);
  EXPECT_EQ(mbp1.levels[1].bid_sz, 31);
  EXPECT_EQ(mbp1.levels[1].ask_sz, 34);
  EXPECT_EQ(mbp1.levels[1].bid_ct, 18);
  EXPECT_EQ(mbp1.levels[1].ask_ct, 24);
  EXPECT_EQ(mbp1.levels[2].bid_px, 3719750000000);
  EXPECT_EQ(mbp1.levels[2].ask_px, 3721000000000);
  EXPECT_EQ(mbp1.levels[2].bid_sz, 32);
  EXPECT_EQ(mbp1.levels[2].ask_sz, 39);
  EXPECT_EQ(mbp1.levels[2].bid_ct, 23);
  EXPECT_EQ(mbp1.levels[2].ask_ct, 25);

  const auto record2 = target_->DecodeRecord();
  ASSERT_NE(record2, nullptr);
  ASSERT_TRUE(record2->Holds<Mbp10Msg>());
  const auto& mbp2 = record2->Get<Mbp10Msg>();
  EXPECT_EQ(mbp2, mbp2);
  EXPECT_EQ(mbp2.hd.publisher_id, 1);
  EXPECT_EQ(mbp2.hd.instrument_id, 5482);
  EXPECT_EQ(mbp2.hd.ts_event.time_since_epoch().count(), 1609160400000435673);
  EXPECT_EQ(mbp2.price, 3720000000000);
  EXPECT_EQ(mbp2.size, 1);
  EXPECT_EQ(mbp2.action, Action::Cancel);
  EXPECT_EQ(mbp2.side, Side::Bid);
  EXPECT_EQ(mbp2.flags.Raw(), 128);
  EXPECT_EQ(mbp2.depth, 1);
  EXPECT_EQ(mbp2.ts_recv.time_since_epoch().count(), 1609160400000750544);
  EXPECT_EQ(mbp2.ts_in_delta.count(), 20625);
  EXPECT_EQ(mbp2.sequence, 1170356);
  EXPECT_EQ(mbp2.levels[0].bid_px, 3720250000000);
  EXPECT_EQ(mbp2.levels[0].ask_px, 3720500000000);
  EXPECT_EQ(mbp2.levels[0].bid_sz, 24);
  EXPECT_EQ(mbp2.levels[0].ask_sz, 10);
  EXPECT_EQ(mbp2.levels[0].bid_ct, 15);
  EXPECT_EQ(mbp2.levels[0].ask_ct, 8);
  EXPECT_EQ(mbp2.levels[1].bid_px, 3720000000000);
  EXPECT_EQ(mbp2.levels[1].ask_px, 3720750000000);
  EXPECT_EQ(mbp2.levels[1].bid_sz, 30);
  EXPECT_EQ(mbp2.levels[1].ask_sz, 34);
  EXPECT_EQ(mbp2.levels[1].bid_ct, 17);
  EXPECT_EQ(mbp2.levels[1].ask_ct, 24);
  EXPECT_EQ(mbp2.levels[2].bid_px, 3719750000000);
  EXPECT_EQ(mbp2.levels[2].ask_px, 3721000000000);
  EXPECT_EQ(mbp2.levels[2].bid_sz, 32);
  EXPECT_EQ(mbp2.levels[2].ask_sz, 39);
  EXPECT_EQ(mbp2.levels[2].bid_ct, 23);
  EXPECT_EQ(mbp2.levels[2].ask_ct, 25);
}

TEST_P(DbnDecoderSchemaTests, TestDecodeCmbp1) {
  const auto [extension, version] = GetParam();
  ReadFromFile("cmbp-1", extension, version);

  const Metadata metadata = target_->DecodeMetadata();
  EXPECT_EQ(metadata.version, version);
  EXPECT_EQ(metadata.dataset, dataset::kGlbxMdp3);
  EXPECT_EQ(metadata.schema, Schema::Cmbp1);
  EXPECT_EQ(metadata.start.time_since_epoch().count(), 1609160400000000000);
  EXPECT_EQ(metadata.end.time_since_epoch().count(), 1609200000000000000);
  EXPECT_EQ(metadata.limit, 2);
  EXPECT_EQ(metadata.stype_in, SType::RawSymbol);
  EXPECT_EQ(metadata.stype_out, SType::InstrumentId);
  EXPECT_EQ(metadata.symbols, std::vector<std::string>{"ESH1"});
  EXPECT_TRUE(metadata.partial.empty());
  EXPECT_TRUE(metadata.not_found.empty());
  AssertMappings(metadata.mappings);

  const auto record1 = target_->DecodeRecord();
  ASSERT_NE(record1, nullptr);
  ASSERT_TRUE(record1->Holds<Cmbp1Msg>());
  const auto& cmbp1 = record1->Get<Cmbp1Msg>();
  EXPECT_EQ(cmbp1.hd.publisher_id, 1);
  EXPECT_EQ(cmbp1.hd.instrument_id, 5482);
  EXPECT_EQ(cmbp1.hd.ts_event.time_since_epoch().count(), 1609160400006001487);
  EXPECT_EQ(cmbp1.price, 3720500000000);
  EXPECT_EQ(cmbp1.size, 1);
  EXPECT_EQ(cmbp1.action, Action::Add);
  EXPECT_EQ(cmbp1.side, Side::Ask);
  EXPECT_EQ(cmbp1.flags.Raw(), 128);
  EXPECT_EQ(cmbp1.ts_recv.time_since_epoch().count(), 1609160400006136329);
  EXPECT_EQ(cmbp1.ts_in_delta.count(), 17214);
  EXPECT_EQ(cmbp1.levels[0].bid_px, 3720250000000);
  EXPECT_EQ(cmbp1.levels[0].ask_px, 3720500000000);
  EXPECT_EQ(cmbp1.levels[0].bid_sz, 24);
  EXPECT_EQ(cmbp1.levels[0].ask_sz, 11);
  EXPECT_EQ(cmbp1.levels[0].bid_pb, 1);
  EXPECT_EQ(cmbp1.levels[0].ask_pb, 1);

  const auto record2 = target_->DecodeRecord();
  ASSERT_NE(record2, nullptr);
  ASSERT_TRUE(record2->Holds<Cmbp1Msg>());
  const auto& cmbp2 = record2->Get<Cmbp1Msg>();
  EXPECT_EQ(cmbp2.hd.publisher_id, 1);
  EXPECT_EQ(cmbp2.hd.instrument_id, 5482);
  EXPECT_EQ(cmbp2.hd.ts_event.time_since_epoch().count(), 1609160400006146661);
  EXPECT_EQ(cmbp2.price, 3720500000000);
  EXPECT_EQ(cmbp2.size, 1);
  EXPECT_EQ(cmbp2.action, Action::Add);
  EXPECT_EQ(cmbp2.side, Side::Ask);
  EXPECT_EQ(cmbp2.flags.Raw(), 128);
  EXPECT_EQ(cmbp2.ts_recv.time_since_epoch().count(), 1609160400006246513);
  EXPECT_EQ(cmbp2.ts_in_delta.count(), 18858);
  EXPECT_EQ(cmbp2.levels[0].bid_px, 3720250000000);
  EXPECT_EQ(cmbp2.levels[0].ask_px, 3720500000000);
  EXPECT_EQ(cmbp2.levels[0].bid_sz, 24);
  EXPECT_EQ(cmbp2.levels[0].ask_sz, 12);
  EXPECT_EQ(cmbp2.levels[0].bid_pb, 1);
  EXPECT_EQ(cmbp2.levels[0].ask_pb, 1);
}

TEST_P(DbnDecoderSchemaTests, TestDecodeCbbo) {
  const auto [extension, version] = GetParam();
  ReadFromFile("cbbo-1s", extension, version);

  const Metadata metadata = target_->DecodeMetadata();
  EXPECT_EQ(metadata.version, version);
  EXPECT_EQ(metadata.dataset, dataset::kGlbxMdp3);
  EXPECT_EQ(metadata.schema, Schema::Cbbo1S);
  EXPECT_EQ(metadata.start.time_since_epoch().count(), 1609160400000000000);
  EXPECT_EQ(metadata.end.time_since_epoch().count(), 1609200000000000000);
  EXPECT_EQ(metadata.limit, 2);
  EXPECT_EQ(metadata.stype_in, SType::RawSymbol);
  EXPECT_EQ(metadata.stype_out, SType::InstrumentId);
  EXPECT_EQ(metadata.symbols, std::vector<std::string>{"ESH1"});
  EXPECT_TRUE(metadata.partial.empty());
  EXPECT_TRUE(metadata.not_found.empty());
  AssertMappings(metadata.mappings);

  const auto record1 = target_->DecodeRecord();
  ASSERT_NE(record1, nullptr);
  ASSERT_TRUE(record1->Holds<CbboMsg>());
  const auto& cbbo1 = record1->Get<CbboMsg>();
  EXPECT_EQ(cbbo1.hd.publisher_id, 1);
  EXPECT_EQ(cbbo1.hd.instrument_id, 5482);
  EXPECT_EQ(cbbo1.hd.ts_event.time_since_epoch().count(), 1609160400006001487);
  EXPECT_EQ(cbbo1.price, 3720500000000);
  EXPECT_EQ(cbbo1.size, 1);
  EXPECT_EQ(cbbo1.side, Side::Ask);
  EXPECT_EQ(cbbo1.flags.Raw(), 128);
  EXPECT_EQ(cbbo1.ts_recv.time_since_epoch().count(), 1609160400006136329);
  EXPECT_EQ(cbbo1.levels[0].bid_px, 3720250000000);
  EXPECT_EQ(cbbo1.levels[0].ask_px, 3720500000000);
  EXPECT_EQ(cbbo1.levels[0].bid_sz, 24);
  EXPECT_EQ(cbbo1.levels[0].ask_sz, 11);
  EXPECT_EQ(cbbo1.levels[0].bid_pb, 1);
  EXPECT_EQ(cbbo1.levels[0].ask_pb, 1);

  const auto record2 = target_->DecodeRecord();
  ASSERT_NE(record2, nullptr);
  ASSERT_TRUE(record2->Holds<CbboMsg>());
  const auto& cbbo2 = record2->Get<CbboMsg>();
  EXPECT_EQ(cbbo2.hd.publisher_id, 1);
  EXPECT_EQ(cbbo2.hd.instrument_id, 5482);
  EXPECT_EQ(cbbo2.hd.ts_event.time_since_epoch().count(), 1609160400006146661);
  EXPECT_EQ(cbbo2.price, 3720500000000);
  EXPECT_EQ(cbbo2.size, 1);
  EXPECT_EQ(cbbo2.side, Side::Ask);
  EXPECT_EQ(cbbo2.flags.Raw(), 128);
  EXPECT_EQ(cbbo2.ts_recv.time_since_epoch().count(), 1609160400006246513);
  EXPECT_EQ(cbbo2.levels[0].bid_px, 3720250000000);
  EXPECT_EQ(cbbo2.levels[0].ask_px, 3720500000000);
  EXPECT_EQ(cbbo2.levels[0].bid_sz, 24);
  EXPECT_EQ(cbbo2.levels[0].ask_sz, 12);
  EXPECT_EQ(cbbo2.levels[0].bid_pb, 1);
  EXPECT_EQ(cbbo2.levels[0].ask_pb, 1);
}

TEST_P(DbnDecoderSchemaTests, TestDecodeTbbo) {
  const auto [extension, version] = GetParam();
  ReadFromFile("tbbo", extension, version);

  const Metadata metadata = target_->DecodeMetadata();
  EXPECT_EQ(metadata.version, version);
  EXPECT_EQ(metadata.dataset, dataset::kGlbxMdp3);
  EXPECT_EQ(metadata.schema, Schema::Tbbo);
  EXPECT_EQ(metadata.start.time_since_epoch().count(), 1609160400000000000);
  EXPECT_EQ(metadata.end.time_since_epoch().count(), 1609200000000000000);
  EXPECT_EQ(metadata.limit, 2);
  EXPECT_EQ(metadata.stype_in, SType::RawSymbol);
  EXPECT_EQ(metadata.stype_out, SType::InstrumentId);
  EXPECT_EQ(metadata.symbols, std::vector<std::string>{"ESH1"});
  EXPECT_TRUE(metadata.partial.empty());
  EXPECT_TRUE(metadata.not_found.empty());
  AssertMappings(metadata.mappings);

  const auto record1 = target_->DecodeRecord();
  ASSERT_NE(record1, nullptr);
  ASSERT_TRUE(record1->Holds<TbboMsg>());
  const auto& tbbo1 = record1->Get<TbboMsg>();
  EXPECT_EQ(tbbo1.hd.publisher_id, 1);
  EXPECT_EQ(tbbo1.hd.instrument_id, 5482);
  EXPECT_EQ(tbbo1.hd.ts_event.time_since_epoch().count(), 1609160400098821953);
  EXPECT_EQ(tbbo1.price, 3720250000000);
  EXPECT_EQ(tbbo1.size, 5);
  EXPECT_EQ(tbbo1.action, Action::Trade);
  EXPECT_EQ(tbbo1.side, Side::Ask);
  EXPECT_EQ(tbbo1.flags.Raw(), 129);
  EXPECT_EQ(tbbo1.depth, 0);
  EXPECT_EQ(tbbo1.ts_recv.time_since_epoch().count(), 1609160400099150057);
  EXPECT_EQ(tbbo1.ts_in_delta.count(), 19251);
  EXPECT_EQ(tbbo1.sequence, 1170380);
  EXPECT_EQ(tbbo1.levels[0].bid_px, 3720250000000);
  EXPECT_EQ(tbbo1.levels[0].ask_px, 3720500000000);
  EXPECT_EQ(tbbo1.levels[0].bid_sz, 26);
  EXPECT_EQ(tbbo1.levels[0].ask_sz, 7);
  EXPECT_EQ(tbbo1.levels[0].bid_ct, 16);
  EXPECT_EQ(tbbo1.levels[0].ask_ct, 6);

  const auto record2 = target_->DecodeRecord();
  ASSERT_NE(record2, nullptr);
  ASSERT_TRUE(record2->Holds<TbboMsg>());
  const auto& tbbo2 = record2->Get<TbboMsg>();
  EXPECT_EQ(tbbo2.hd.publisher_id, 1);
  EXPECT_EQ(tbbo2.hd.instrument_id, 5482);
  EXPECT_EQ(tbbo2.hd.ts_event.time_since_epoch().count(), 1609160400107665963);
  EXPECT_EQ(tbbo2.price, 3720250000000);
  EXPECT_EQ(tbbo2.size, 21);
  EXPECT_EQ(tbbo2.action, Action::Trade);
  EXPECT_EQ(tbbo2.side, Side::Ask);
  EXPECT_EQ(tbbo2.flags.Raw(), 129);
  EXPECT_EQ(tbbo2.depth, 0);
  EXPECT_EQ(tbbo2.ts_recv.time_since_epoch().count(), 1609160400108142648);
  EXPECT_EQ(tbbo2.ts_in_delta.count(), 20728);
  EXPECT_EQ(tbbo2.sequence, 1170414);
  EXPECT_EQ(tbbo2.levels[0].bid_px, 3720250000000);
  EXPECT_EQ(tbbo2.levels[0].ask_px, 3720500000000);
  EXPECT_EQ(tbbo2.levels[0].bid_sz, 21);
  EXPECT_EQ(tbbo2.levels[0].ask_sz, 22);
  EXPECT_EQ(tbbo2.levels[0].bid_ct, 13);
  EXPECT_EQ(tbbo2.levels[0].ask_ct, 15);
}

TEST_P(DbnDecoderSchemaTests, TestDecodeTrades) {
  const auto [extension, version] = GetParam();
  ReadFromFile("trades", extension, version);

  const Metadata metadata = target_->DecodeMetadata();
  EXPECT_EQ(metadata.version, version);
  EXPECT_EQ(metadata.dataset, dataset::kGlbxMdp3);
  EXPECT_EQ(metadata.schema, Schema::Trades);
  EXPECT_EQ(metadata.start.time_since_epoch().count(), 1609160400000000000);
  EXPECT_EQ(metadata.end.time_since_epoch().count(), 1609200000000000000);
  EXPECT_EQ(metadata.limit, 2);
  EXPECT_EQ(metadata.stype_in, SType::RawSymbol);
  EXPECT_EQ(metadata.stype_out, SType::InstrumentId);
  EXPECT_EQ(metadata.symbols, std::vector<std::string>{"ESH1"});
  EXPECT_TRUE(metadata.partial.empty());
  EXPECT_TRUE(metadata.not_found.empty());
  AssertMappings(metadata.mappings);

  const auto record1 = target_->DecodeRecord();
  ASSERT_NE(record1, nullptr);
  ASSERT_TRUE(record1->Holds<TradeMsg>());
  const auto& trade1 = record1->Get<TradeMsg>();
  EXPECT_EQ(trade1.hd.publisher_id, 1);
  EXPECT_EQ(trade1.hd.instrument_id, 5482);
  EXPECT_EQ(trade1.hd.ts_event.time_since_epoch().count(), 1609160400098821953);
  EXPECT_EQ(trade1.price, 3720250000000);
  EXPECT_EQ(trade1.size, 5);
  EXPECT_EQ(trade1.action, Action::Trade);
  EXPECT_EQ(trade1.side, Side::Ask);
  EXPECT_EQ(trade1.flags.Raw(), 129);
  EXPECT_EQ(trade1.depth, 0);
  EXPECT_EQ(trade1.ts_recv.time_since_epoch().count(), 1609160400099150057);
  EXPECT_EQ(trade1.ts_in_delta.count(), 19251);
  EXPECT_EQ(trade1.sequence, 1170380);

  const auto record2 = target_->DecodeRecord();
  ASSERT_NE(record2, nullptr);
  ASSERT_TRUE(record2->Holds<TradeMsg>());
  const auto& trade2 = record2->Get<TradeMsg>();
  EXPECT_EQ(trade2.hd.publisher_id, 1);
  EXPECT_EQ(trade2.hd.instrument_id, 5482);
  EXPECT_EQ(trade2.hd.ts_event.time_since_epoch().count(), 1609160400107665963);
  EXPECT_EQ(trade2.price, 3720250000000);
  EXPECT_EQ(trade2.size, 21);
  EXPECT_EQ(trade2.action, Action::Trade);
  EXPECT_EQ(trade2.side, Side::Ask);
  EXPECT_EQ(trade2.flags.Raw(), 129);
  EXPECT_EQ(trade2.depth, 0);
  EXPECT_EQ(trade2.ts_recv.time_since_epoch().count(), 1609160400108142648);
  EXPECT_EQ(trade2.ts_in_delta.count(), 20728);
  EXPECT_EQ(trade2.sequence, 1170414);
}

TEST_P(DbnDecoderSchemaTests, TestDecodeOhlcv1D) {
  const auto [extension, version] = GetParam();
  ReadFromFile("ohlcv-1d", extension, version);

  const Metadata metadata = target_->DecodeMetadata();
  EXPECT_EQ(metadata.version, version);
  EXPECT_EQ(metadata.dataset, dataset::kGlbxMdp3);
  EXPECT_EQ(metadata.schema, Schema::Ohlcv1D);
  EXPECT_EQ(metadata.start.time_since_epoch().count(), 1609160400000000000);
  EXPECT_EQ(metadata.end.time_since_epoch().count(), 1609200000000000000);
  EXPECT_EQ(metadata.limit, 2);
  EXPECT_EQ(metadata.stype_in, SType::RawSymbol);
  EXPECT_EQ(metadata.stype_out, SType::InstrumentId);
  EXPECT_EQ(metadata.symbols, std::vector<std::string>{"ESH1"});
  EXPECT_TRUE(metadata.partial.empty());
  EXPECT_TRUE(metadata.not_found.empty());
  AssertMappings(metadata.mappings);
}

TEST_P(DbnDecoderSchemaTests, TestDecodeOhlcv1H) {
  const auto [extension, version] = GetParam();
  ReadFromFile("ohlcv-1h", extension, version);

  const Metadata metadata = target_->DecodeMetadata();
  EXPECT_EQ(metadata.version, version);
  EXPECT_EQ(metadata.dataset, dataset::kGlbxMdp3);
  EXPECT_EQ(metadata.schema, Schema::Ohlcv1H);
  EXPECT_EQ(metadata.start.time_since_epoch().count(), 1609160400000000000);
  EXPECT_EQ(metadata.end.time_since_epoch().count(), 1609200000000000000);
  EXPECT_EQ(metadata.limit, 2);
  EXPECT_EQ(metadata.stype_in, SType::RawSymbol);
  EXPECT_EQ(metadata.stype_out, SType::InstrumentId);
  EXPECT_EQ(metadata.symbols, std::vector<std::string>{"ESH1"});
  EXPECT_TRUE(metadata.partial.empty());
  EXPECT_TRUE(metadata.not_found.empty());
  AssertMappings(metadata.mappings);

  const auto record1 = target_->DecodeRecord();
  ASSERT_NE(record1, nullptr);
  ASSERT_TRUE(record1->Holds<OhlcvMsg>());
  const auto& ohlcv1 = record1->Get<OhlcvMsg>();
  EXPECT_EQ(ohlcv1.hd.publisher_id, 1);
  EXPECT_EQ(ohlcv1.hd.instrument_id, 5482);
  EXPECT_EQ(ohlcv1.hd.ts_event.time_since_epoch().count(), 1609160400000000000);
  EXPECT_EQ(ohlcv1.open, 372025000000000);
  EXPECT_EQ(ohlcv1.high, 372350000000000);
  EXPECT_EQ(ohlcv1.low, 372025000000000);
  EXPECT_EQ(ohlcv1.close, 372225000000000);
  EXPECT_EQ(ohlcv1.volume, 9385);

  const auto record2 = target_->DecodeRecord();
  ASSERT_NE(record2, nullptr);
  ASSERT_TRUE(record2->Holds<OhlcvMsg>());
  const auto& ohlcv2 = record2->Get<OhlcvMsg>();
  EXPECT_EQ(ohlcv2.hd.publisher_id, 1);
  EXPECT_EQ(ohlcv2.hd.instrument_id, 5482);
  EXPECT_EQ(ohlcv2.hd.ts_event.time_since_epoch().count(), 1609164000000000000);
  EXPECT_EQ(ohlcv2.open, 372225000000000);
  EXPECT_EQ(ohlcv2.high, 372450000000000);
  EXPECT_EQ(ohlcv2.low, 371600000000000);
  EXPECT_EQ(ohlcv2.close, 371950000000000);
  EXPECT_EQ(ohlcv2.volume, 112698);
}

TEST_P(DbnDecoderSchemaTests, TestDecodeOhlcv1M) {
  const auto [extension, version] = GetParam();
  ReadFromFile("ohlcv-1m", extension, version);

  const Metadata metadata = target_->DecodeMetadata();
  EXPECT_EQ(metadata.version, version);
  EXPECT_EQ(metadata.dataset, dataset::kGlbxMdp3);
  EXPECT_EQ(metadata.schema, Schema::Ohlcv1M);
  EXPECT_EQ(metadata.start.time_since_epoch().count(), 1609160400000000000);
  EXPECT_EQ(metadata.end.time_since_epoch().count(), 1609200000000000000);
  EXPECT_EQ(metadata.limit, 2);
  EXPECT_EQ(metadata.stype_in, SType::RawSymbol);
  EXPECT_EQ(metadata.stype_out, SType::InstrumentId);
  EXPECT_EQ(metadata.symbols, std::vector<std::string>{"ESH1"});
  EXPECT_TRUE(metadata.partial.empty());
  EXPECT_TRUE(metadata.not_found.empty());
  AssertMappings(metadata.mappings);

  const auto record1 = target_->DecodeRecord();
  ASSERT_NE(record1, nullptr);
  ASSERT_TRUE(record1->Holds<OhlcvMsg>());
  const auto& ohlcv1 = record1->Get<OhlcvMsg>();
  EXPECT_EQ(ohlcv1.hd.publisher_id, 1);
  EXPECT_EQ(ohlcv1.hd.instrument_id, 5482);
  EXPECT_EQ(ohlcv1.hd.ts_event.time_since_epoch().count(), 1609160400000000000);
  EXPECT_EQ(ohlcv1.open, 372025000000000);
  EXPECT_EQ(ohlcv1.high, 372150000000000);
  EXPECT_EQ(ohlcv1.low, 372025000000000);
  EXPECT_EQ(ohlcv1.close, 372100000000000);
  EXPECT_EQ(ohlcv1.volume, 353);

  const auto record2 = target_->DecodeRecord();
  ASSERT_NE(record2, nullptr);
  ASSERT_TRUE(record2->Holds<OhlcvMsg>());
  const auto& ohlcv2 = record2->Get<OhlcvMsg>();
  EXPECT_EQ(ohlcv2.hd.publisher_id, 1);
  EXPECT_EQ(ohlcv2.hd.instrument_id, 5482);
  EXPECT_EQ(ohlcv2.hd.ts_event.time_since_epoch().count(), 1609160460000000000);
  EXPECT_EQ(ohlcv2.open, 372100000000000);
  EXPECT_EQ(ohlcv2.high, 372150000000000);
  EXPECT_EQ(ohlcv2.low, 372100000000000);
  EXPECT_EQ(ohlcv2.close, 372150000000000);
  EXPECT_EQ(ohlcv2.volume, 152);
}

TEST_P(DbnDecoderSchemaTests, TestDecodeOhlcv1S) {
  const auto [extension, version] = GetParam();
  ReadFromFile("ohlcv-1s", extension, version);

  const Metadata metadata = target_->DecodeMetadata();
  EXPECT_EQ(metadata.version, version);
  EXPECT_EQ(metadata.dataset, dataset::kGlbxMdp3);
  EXPECT_EQ(metadata.schema, Schema::Ohlcv1S);
  EXPECT_EQ(metadata.start.time_since_epoch().count(), 1609160400000000000);
  EXPECT_EQ(metadata.end.time_since_epoch().count(), 1609200000000000000);
  EXPECT_EQ(metadata.limit, 2);
  EXPECT_EQ(metadata.stype_in, SType::RawSymbol);
  EXPECT_EQ(metadata.stype_out, SType::InstrumentId);
  EXPECT_EQ(metadata.symbols, std::vector<std::string>{"ESH1"});
  EXPECT_TRUE(metadata.partial.empty());
  EXPECT_TRUE(metadata.not_found.empty());
  AssertMappings(metadata.mappings);

  const auto record1 = target_->DecodeRecord();
  ASSERT_NE(record1, nullptr);
  ASSERT_TRUE(record1->Holds<OhlcvMsg>());
  const auto& ohlcv1 = record1->Get<OhlcvMsg>();
  EXPECT_EQ(ohlcv1.hd.publisher_id, 1);
  EXPECT_EQ(ohlcv1.hd.instrument_id, 5482);
  EXPECT_EQ(ohlcv1.hd.ts_event.time_since_epoch().count(), 1609160400000000000);
  EXPECT_EQ(ohlcv1.open, 372025000000000);
  EXPECT_EQ(ohlcv1.high, 372050000000000);
  EXPECT_EQ(ohlcv1.low, 372025000000000);
  EXPECT_EQ(ohlcv1.close, 372050000000000);
  EXPECT_EQ(ohlcv1.volume, 57);

  const auto record2 = target_->DecodeRecord();
  ASSERT_NE(record2, nullptr);
  ASSERT_TRUE(record2->Holds<OhlcvMsg>());
  const auto& ohlcv2 = record2->Get<OhlcvMsg>();
  EXPECT_EQ(ohlcv2.hd.publisher_id, 1);
  EXPECT_EQ(ohlcv2.hd.instrument_id, 5482);
  EXPECT_EQ(ohlcv2.hd.ts_event.time_since_epoch().count(), 1609160401000000000);
  EXPECT_EQ(ohlcv2.open, 372050000000000);
  EXPECT_EQ(ohlcv2.high, 372050000000000);
  EXPECT_EQ(ohlcv2.low, 372050000000000);
  EXPECT_EQ(ohlcv2.close, 372050000000000);
  EXPECT_EQ(ohlcv2.volume, 13);
}

TEST_P(DbnDecoderSchemaTests, TestDecodeDefinition) {
  const auto [extension, version] = GetParam();
  ReadFromFile("definition", extension, version);

  const Metadata metadata = target_->DecodeMetadata();
  EXPECT_EQ(metadata.version, version);
  EXPECT_EQ(metadata.dataset, dataset::kXnasItch);
  EXPECT_EQ(metadata.schema, Schema::Definition);
  EXPECT_EQ(metadata.start.time_since_epoch().count(), 1633305600000000000);
  EXPECT_EQ(metadata.end.time_since_epoch().count(), 1641254400000000000);
  EXPECT_EQ(metadata.limit, 2);
  EXPECT_EQ(metadata.stype_in, SType::RawSymbol);
  EXPECT_EQ(metadata.stype_out, SType::InstrumentId);
  EXPECT_EQ(metadata.symbols, std::vector<std::string>{"MSFT"});
  EXPECT_TRUE(metadata.partial.empty());
  EXPECT_TRUE(metadata.not_found.empty());
  EXPECT_EQ(metadata.mappings.size(), 1);
  const auto& mapping = metadata.mappings.at(0);
  EXPECT_EQ(mapping.raw_symbol, "MSFT");
  ASSERT_EQ(mapping.intervals.size(), 62);
  const auto& interval = mapping.intervals.at(0);
  EXPECT_EQ(interval.symbol, "6819");
  EXPECT_EQ(interval.start_date, date::year{2021} / 10 / 4);
  EXPECT_EQ(interval.end_date, date::year{2021} / 10 / 5);

  const auto record1 = target_->DecodeRecord();
  ASSERT_NE(record1, nullptr);
  ASSERT_NE(record1, nullptr);

  const auto record2 = target_->DecodeRecord();
  ASSERT_NE(record2, nullptr);
  ASSERT_NE(record2, nullptr);
  if (version == 1) {
    AssertDefHas<v1::InstrumentDefMsg>(record1);
    AssertDefHas<v1::InstrumentDefMsg>(record2);
  } else if (version == 2) {
    AssertDefHas<v2::InstrumentDefMsg>(record1);
    AssertDefHas<v2::InstrumentDefMsg>(record2);
  } else {
    AssertDefHas<v3::InstrumentDefMsg>(record1);
    AssertDefHas<v3::InstrumentDefMsg>(record2);
  }
}

TEST_P(DbnDecoderSchemaTests, TestDecodeImbalance) {
  const auto [extension, version] = GetParam();
  ReadFromFile("imbalance", extension, version);

  const Metadata metadata = target_->DecodeMetadata();
  EXPECT_EQ(metadata.version, version);
  EXPECT_EQ(metadata.dataset, dataset::kXnasItch);
  EXPECT_EQ(metadata.schema, Schema::Imbalance);
  EXPECT_EQ(metadata.start.time_since_epoch().count(), 1633305600000000000);
  EXPECT_EQ(metadata.end.time_since_epoch().count(), 1641254400000000000);
  EXPECT_EQ(metadata.limit, 2);
  EXPECT_EQ(metadata.stype_in, SType::RawSymbol);
  EXPECT_EQ(metadata.stype_out, SType::InstrumentId);
  EXPECT_EQ(metadata.symbols, std::vector<std::string>{"SPOT"});
  EXPECT_TRUE(metadata.partial.empty());
  EXPECT_TRUE(metadata.not_found.empty());
  EXPECT_EQ(metadata.mappings.size(), 1);

  const auto record1 = target_->DecodeRecord();
  ASSERT_NE(record1, nullptr);
  ASSERT_TRUE(record1->Holds<ImbalanceMsg>());
  const auto& imbalance1 = record1->Get<ImbalanceMsg>();
  EXPECT_EQ(imbalance1, imbalance1);
  EXPECT_EQ(imbalance1.ref_price, 229430000000);

  const auto record2 = target_->DecodeRecord();
  ASSERT_NE(record2, nullptr);
  ASSERT_TRUE(record2->Holds<ImbalanceMsg>());
  const auto& imbalance2 = record2->Get<ImbalanceMsg>();
  EXPECT_EQ(imbalance2.ref_price, 229990000000);
}

TEST_P(DbnDecoderSchemaTests, TestDecodeStatistics) {
  const auto [extension, version] = GetParam();
  ReadFromFile("statistics", extension, version);

  const Metadata metadata = target_->DecodeMetadata();
  EXPECT_EQ(metadata.version, version);
  EXPECT_EQ(metadata.dataset, dataset::kGlbxMdp3);
  EXPECT_EQ(metadata.schema, Schema::Statistics);
  EXPECT_EQ(metadata.start.time_since_epoch().count(), 2814749767106560);
  EXPECT_EQ(metadata.end.time_since_epoch().count(), 18446744073709551615UL);
  EXPECT_EQ(metadata.limit, 2);
  EXPECT_EQ(metadata.stype_in, SType::InstrumentId);
  EXPECT_EQ(metadata.stype_out, SType::InstrumentId);
  EXPECT_TRUE(metadata.symbols.empty());
  EXPECT_TRUE(metadata.partial.empty());
  EXPECT_TRUE(metadata.not_found.empty());
  EXPECT_TRUE(metadata.mappings.empty());

  const auto record1 = target_->DecodeRecord();
  if (version < 3) {
    AssertStatHas<v1::StatMsg>(record1, StatType::LowestOffer, 100 * kFixedPriceScale,
                               v1::kUndefStatQuantity);
  } else {
    AssertStatHas<v3::StatMsg>(record1, StatType::LowestOffer, 100 * kFixedPriceScale,
                               v3::kUndefStatQuantity);
  }

  const auto record2 = target_->DecodeRecord();
  if (version < 3) {
    AssertStatHas<v1::StatMsg>(record2, StatType::TradingSessionHighPrice,
                               100 * kFixedPriceScale, v1::kUndefStatQuantity);
  } else {
    AssertStatHas<v3::StatMsg>(record2, StatType::TradingSessionHighPrice,
                               100 * kFixedPriceScale, v3::kUndefStatQuantity);
  }
}

class DbnIdentityTests
    : public testing::TestWithParam<std::tuple<std::uint8_t, Schema, Compression>> {
 protected:
  mock::MockLogReceiver logger_ =
      mock::MockLogReceiver::AssertNoLogs(LogLevel::Warning);
};

INSTANTIATE_TEST_SUITE_P(
    TestFiles, DbnIdentityTests,
    testing::Values(std::make_tuple(1, Schema::Mbo, Compression::Zstd),
                    std::make_tuple(1, Schema::Trades, Compression::Zstd),
                    std::make_tuple(1, Schema::Mbp1, Compression::Zstd),
                    std::make_tuple(1, Schema::Tbbo, Compression::Zstd),
                    std::make_tuple(1, Schema::Mbp10, Compression::Zstd),
                    std::make_tuple(1, Schema::Ohlcv1D, Compression::Zstd),
                    std::make_tuple(1, Schema::Ohlcv1H, Compression::Zstd),
                    std::make_tuple(1, Schema::Ohlcv1M, Compression::Zstd),
                    std::make_tuple(1, Schema::Ohlcv1S, Compression::Zstd),
                    std::make_tuple(1, Schema::Definition, Compression::Zstd),
                    std::make_tuple(1, Schema::Imbalance, Compression::Zstd),
                    std::make_tuple(1, Schema::Statistics, Compression::Zstd),
                    std::make_tuple(1, Schema::Cmbp1, Compression::Zstd),
                    std::make_tuple(1, Schema::Cbbo1S, Compression::Zstd),
                    std::make_tuple(2, Schema::Mbo, Compression::Zstd),
                    std::make_tuple(2, Schema::Trades, Compression::Zstd),
                    std::make_tuple(2, Schema::Tbbo, Compression::Zstd),
                    std::make_tuple(2, Schema::Mbp1, Compression::Zstd),
                    std::make_tuple(2, Schema::Mbp10, Compression::Zstd),
                    std::make_tuple(2, Schema::Ohlcv1D, Compression::Zstd),
                    std::make_tuple(2, Schema::Ohlcv1H, Compression::Zstd),
                    std::make_tuple(2, Schema::Ohlcv1M, Compression::Zstd),
                    std::make_tuple(2, Schema::Ohlcv1S, Compression::Zstd),
                    std::make_tuple(2, Schema::Definition, Compression::Zstd),
                    std::make_tuple(2, Schema::Imbalance, Compression::Zstd),
                    std::make_tuple(2, Schema::Statistics, Compression::Zstd),
                    std::make_tuple(2, Schema::Bbo1S, Compression::Zstd),
                    std::make_tuple(2, Schema::Bbo1M, Compression::Zstd),
                    std::make_tuple(2, Schema::Cmbp1, Compression::Zstd),
                    std::make_tuple(2, Schema::Cbbo1S, Compression::Zstd),
                    std::make_tuple(2, Schema::Status, Compression::Zstd),
                    std::make_tuple(3, Schema::Mbo, Compression::None),
                    std::make_tuple(3, Schema::Mbo, Compression::Zstd),
                    std::make_tuple(3, Schema::Trades, Compression::Zstd),
                    std::make_tuple(3, Schema::Tbbo, Compression::Zstd),
                    std::make_tuple(3, Schema::Mbp1, Compression::Zstd),
                    std::make_tuple(3, Schema::Mbp10, Compression::Zstd),
                    std::make_tuple(3, Schema::Ohlcv1D, Compression::Zstd),
                    std::make_tuple(3, Schema::Ohlcv1H, Compression::Zstd),
                    std::make_tuple(3, Schema::Ohlcv1M, Compression::Zstd),
                    std::make_tuple(3, Schema::Ohlcv1S, Compression::Zstd),
                    std::make_tuple(3, Schema::Definition, Compression::Zstd),
                    std::make_tuple(3, Schema::Imbalance, Compression::Zstd),
                    std::make_tuple(3, Schema::Statistics, Compression::Zstd),
                    std::make_tuple(3, Schema::Bbo1S, Compression::Zstd),
                    std::make_tuple(3, Schema::Bbo1M, Compression::Zstd),
                    std::make_tuple(3, Schema::Cmbp1, Compression::Zstd),
                    std::make_tuple(3, Schema::Cbbo1S, Compression::Zstd),
                    std::make_tuple(3, Schema::Status, Compression::Zstd)),
    [](const testing::TestParamInfo<std::tuple<std::uint8_t, Schema, Compression>>&
           test_info) {
      const auto version = std::get<0>(test_info.param);
      const auto schema = std::get<1>(test_info.param);
      const auto compression = std::get<2>(test_info.param);
      std::string schema_str = ToString(schema);
      for (auto& c : schema_str) {
        if (c == '-') {
          c = '_';
        }
      }
      return schema_str + "_" + ToString(compression) + "_DBNv" +
             std::to_string(version);
    });

TEST_P(DbnIdentityTests, TestIdentity) {
  const auto [version, schema, compression] = GetParam();
  const auto file_name = std::string{TEST_DATA_DIR "/test_data."} + ToString(schema) +
                         ".v" + std::to_string(+version) +
                         (compression == Compression::Zstd ? ".dbn.zst" : ".dbn");
  DbnDecoder file_decoder{&logger_, std::make_unique<InFileStream>(file_name),
                          VersionUpgradePolicy::AsIs};
  const Metadata file_metadata = file_decoder.DecodeMetadata();

  detail::Buffer buf_io;
  {
    std::unique_ptr<detail::ZstdCompressStream> zstd_io;
    if (compression == Compression::Zstd) {
      zstd_io = std::make_unique<detail::ZstdCompressStream>(&buf_io);
    }
    DbnEncoder encoder{file_metadata,
                       zstd_io ? static_cast<IWritable*>(zstd_io.get()) : &buf_io};
    while (auto* record = file_decoder.DecodeRecord()) {
      encoder.EncodeRecord(*record);
    }
    // Free zstd_io and flush
  }

  file_decoder = {&logger_, std::make_unique<InFileStream>(file_name),
                  VersionUpgradePolicy::AsIs};
  file_decoder.DecodeMetadata();

  auto input = std::make_unique<detail::Buffer>(std::move(buf_io));
  DbnDecoder buf_decoder{&logger_, std::move(input), VersionUpgradePolicy::AsIs};
  const auto buf_metadata = buf_decoder.DecodeMetadata();
  EXPECT_EQ(file_metadata, buf_metadata);
  while (auto* buf_record = buf_decoder.DecodeRecord()) {
    auto* file_record = file_decoder.DecodeRecord();
    ASSERT_NE(file_record, nullptr);
    if (auto* mbo = buf_record->GetIf<MboMsg>()) {
      EXPECT_EQ(*mbo, file_record->Get<MboMsg>());
    } else if (auto* trade = buf_record->GetIf<TradeMsg>()) {
      EXPECT_EQ(*trade, file_record->Get<TradeMsg>());
    } else if (auto* mbp1 = buf_record->GetIf<Mbp1Msg>()) {
      EXPECT_EQ(*mbp1, file_record->Get<Mbp1Msg>());
    } else if (auto* mbp10 = buf_record->GetIf<Mbp10Msg>()) {
      EXPECT_EQ(*mbp10, file_record->Get<Mbp10Msg>());
    } else if (auto* cmbp = buf_record->GetIf<Cmbp1Msg>()) {
      EXPECT_EQ(*cmbp, file_record->Get<Cmbp1Msg>());
    } else if (auto* bbo = buf_record->GetIf<BboMsg>()) {
      EXPECT_EQ(*bbo, file_record->Get<BboMsg>());
    } else if (auto* cbbo = buf_record->GetIf<CbboMsg>()) {
      EXPECT_EQ(*cbbo, file_record->Get<CbboMsg>());
    } else if (auto* status = buf_record->GetIf<StatusMsg>()) {
      EXPECT_EQ(*status, file_record->Get<StatusMsg>());
    } else if (auto* ohlcv = buf_record->GetIf<OhlcvMsg>()) {
      EXPECT_EQ(*ohlcv, file_record->Get<OhlcvMsg>());
    } else if (auto* trade = buf_record->GetIf<TradeMsg>()) {
      EXPECT_EQ(*trade, file_record->Get<TradeMsg>());
    } else if (auto* imbalance = buf_record->GetIf<ImbalanceMsg>()) {
      EXPECT_EQ(*imbalance, file_record->Get<ImbalanceMsg>());
    } else if (buf_record->Header().rtype == RType::InstrumentDef) {
      if (buf_record->Size() == sizeof(v1::InstrumentDefMsg)) {
        EXPECT_EQ(buf_record->Get<v1::InstrumentDefMsg>(),
                  file_record->Get<v1::InstrumentDefMsg>());
      } else if (buf_record->Size() == sizeof(v2::InstrumentDefMsg)) {
        EXPECT_EQ(buf_record->Get<v2::InstrumentDefMsg>(),
                  file_record->Get<v2::InstrumentDefMsg>());
      } else if (buf_record->Size() == sizeof(v3::InstrumentDefMsg)) {
        EXPECT_EQ(buf_record->Get<v3::InstrumentDefMsg>(),
                  file_record->Get<v3::InstrumentDefMsg>());
      } else {
        FAIL() << "Unknown definition size";
      }
    } else if (buf_record->Header().rtype == RType::Statistics) {
      if (buf_record->Size() == sizeof(v1::StatMsg)) {
        EXPECT_EQ(buf_record->Get<v1::StatMsg>(), file_record->Get<v1::StatMsg>());
      } else if (buf_record->Size() == sizeof(v3::StatMsg)) {
        EXPECT_EQ(buf_record->Get<v3::StatMsg>(), file_record->Get<v3::StatMsg>());
      } else {
        FAIL() << "Unknown stats size";
      }
    } else {
      FAIL() << "Unexpected rtype "
             << static_cast<std::uint16_t>(file_record->Header().rtype);
    }
  }
  ASSERT_EQ(file_decoder.DecodeRecord(), nullptr);
}
}  // namespace databento::tests
