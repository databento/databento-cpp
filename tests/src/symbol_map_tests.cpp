#include <date/date.h>
#include <gtest/gtest.h>

#include <chrono>
#include <cstdint>
#include <cstring>
#include <string>
#include <unordered_map>

#include "databento/compat.hpp"
#include "databento/constants.hpp"
#include "databento/datetime.hpp"
#include "databento/dbn.hpp"
#include "databento/exceptions.hpp"
#include "databento/publishers.hpp"
#include "databento/record.hpp"
#include "databento/symbol_map.hpp"

namespace databento::tests {
Metadata GenMetadata() {
  Metadata metadata{
      kDbnVersion,
      ToString(Dataset::XnasItch),
      Schema::Trades,
      {date::sys_days{date::year{2023} / 7 / 1}},
      {date::sys_days{date::year{2023} / 8 / 1}},
      {},
      SType::RawSymbol,
      SType::InstrumentId,
      false,
      kSymbolCstrLen,
      {},
      {},
      {},
      {{"AAPL", {{date::year{2023} / 7 / 1, date::year{2023} / 8 / 1, "32"}}},
       {"TSLA",
        {{date::year{2023} / 7 / 1, date::year{2023} / 7 / 3, "10221"},
         {date::year{2023} / 7 / 3, date::year{2023} / 7 / 5, "10213"},
         {date::year{2023} / 7 / 5, date::year{2023} / 7 / 6, "10209"},
         {date::year{2023} / 7 / 6, date::year{2023} / 7 / 7, "10206"},
         {date::year{2023} / 7 / 7, date::year{2023} / 7 / 10, "10201"},
         {date::year{2023} / 7 / 10, date::year{2023} / 7 / 11, "10193"},
         {date::year{2023} / 7 / 11, date::year{2023} / 7 / 12, "10192"},
         {date::year{2023} / 7 / 12, date::year{2023} / 7 / 13, "10189"},
         {date::year{2023} / 7 / 13, date::year{2023} / 7 / 14, "10191"},
         {date::year{2023} / 7 / 14, date::year{2023} / 7 / 17, "10188"},
         {date::year{2023} / 7 / 17, date::year{2023} / 7 / 20, "10186"},
         {date::year{2023} / 7 / 20, date::year{2023} / 7 / 21, "10184"},
         {date::year{2023} / 7 / 21, date::year{2023} / 7 / 24, "10181"},
         {date::year{2023} / 7 / 24, date::year{2023} / 7 / 25, "10174"},
         {date::year{2023} / 7 / 25, date::year{2023} / 7 / 26, "10172"},
         {date::year{2023} / 7 / 26, date::year{2023} / 7 / 27, "10169"},
         {date::year{2023} / 7 / 27, date::year{2023} / 7 / 28, "10168"},
         {date::year{2023} / 7 / 28, date::year{2023} / 7 / 31, "10164"},
         {date::year{2023} / 7 / 31, date::year{2023} / 8 / 1, "10163"}}},
       {"MSFT",
        {
            {date::year{2023} / 7 / 1, date::year{2023} / 7 / 3, "6854"},
            {date::year{2023} / 7 / 3, date::year{2023} / 7 / 5, "6849"},
            {date::year{2023} / 7 / 5, date::year{2023} / 7 / 6, "6846"},
            {date::year{2023} / 7 / 6, date::year{2023} / 7 / 7, "6843"},
            {date::year{2023} / 7 / 7, date::year{2023} / 7 / 10, "6840"},
            {date::year{2023} / 7 / 10, date::year{2023} / 7 / 11, "6833"},
            {date::year{2023} / 7 / 11, date::year{2023} / 7 / 12, "6830"},
            {date::year{2023} / 7 / 12, date::year{2023} / 7 / 13, "6826"},
            {date::year{2023} / 7 / 13, date::year{2023} / 7 / 17, "6827"},
            {date::year{2023} / 7 / 17, date::year{2023} / 7 / 18, "6824"},
            {date::year{2023} / 7 / 18, date::year{2023} / 7 / 19, "6823"},
            {date::year{2023} / 7 / 19, date::year{2023} / 7 / 20, "6822"},
            {date::year{2023} / 7 / 20, date::year{2023} / 7 / 21, "6818"},
            {date::year{2023} / 7 / 21, date::year{2023} / 7 / 24, "6815"},
            {date::year{2023} / 7 / 24, date::year{2023} / 7 / 25, "6814"},
            {date::year{2023} / 7 / 25, date::year{2023} / 7 / 26, "6812"},
            {date::year{2023} / 7 / 26, date::year{2023} / 7 / 27, "6810"},
            {date::year{2023} / 7 / 27, date::year{2023} / 7 / 28, "6808"},
            {date::year{2023} / 7 / 28, date::year{2023} / 7 / 31, "6805"},
            {date::year{2023} / 7 / 31, date::year{2023} / 8 / 1, "6803"},
        }},
       {"NVDA",
        {{date::year{2023} / 7 / 1, date::year{2023} / 7 / 3, "7348"},
         {date::year{2023} / 7 / 3, date::year{2023} / 7 / 5, "7343"},
         {date::year{2023} / 7 / 5, date::year{2023} / 7 / 6, "7340"},
         {date::year{2023} / 7 / 6, date::year{2023} / 7 / 7, "7337"},
         {date::year{2023} / 7 / 7, date::year{2023} / 7 / 10, "7335"},
         {date::year{2023} / 7 / 10, date::year{2023} / 7 / 11, "7328"},
         {date::year{2023} / 7 / 11, date::year{2023} / 7 / 12, "7325"},
         {date::year{2023} / 7 / 12, date::year{2023} / 7 / 13, "7321"},
         {date::year{2023} / 7 / 13, date::year{2023} / 7 / 17, "7322"},
         {date::year{2023} / 7 / 17, date::year{2023} / 7 / 18, "7320"},
         {date::year{2023} / 7 / 18, date::year{2023} / 7 / 19, "7319"},
         {date::year{2023} / 7 / 19, date::year{2023} / 7 / 20, "7318"},
         {date::year{2023} / 7 / 20, date::year{2023} / 7 / 21, "7314"},
         {date::year{2023} / 7 / 21, date::year{2023} / 7 / 24, "7311"},
         {date::year{2023} / 7 / 24, date::year{2023} / 7 / 25, "7310"},
         {date::year{2023} / 7 / 25, date::year{2023} / 7 / 26, "7308"},
         {date::year{2023} / 7 / 26, date::year{2023} / 7 / 27, "7303"},
         {date::year{2023} / 7 / 27, date::year{2023} / 7 / 28, "7301"},
         {date::year{2023} / 7 / 28, date::year{2023} / 7 / 31, "7298"},
         {date::year{2023} / 7 / 31, date::year{2023} / 8 / 1, "7295"}}},
       {"PLTR",
        {{date::year{2023} / 7 / 1, date::year{2023} / 7 / 3, "8043"},
         {date::year{2023} / 7 / 3, date::year{2023} / 7 / 5, "8038"},
         {date::year{2023} / 7 / 5, date::year{2023} / 7 / 6, "8035"},
         {date::year{2023} / 7 / 6, date::year{2023} / 7 / 7, "8032"},
         {date::year{2023} / 7 / 7, date::year{2023} / 7 / 10, "8029"},
         {date::year{2023} / 7 / 10, date::year{2023} / 7 / 11, "8022"},
         {date::year{2023} / 7 / 11, date::year{2023} / 7 / 12, "8019"},
         {date::year{2023} / 7 / 12, date::year{2023} / 7 / 13, "8015"},
         {date::year{2023} / 7 / 13, date::year{2023} / 7 / 17, "8016"},
         {date::year{2023} / 7 / 17, date::year{2023} / 7 / 19, "8014"},
         {date::year{2023} / 7 / 19, date::year{2023} / 7 / 20, "8013"},
         {date::year{2023} / 7 / 20, date::year{2023} / 7 / 21, "8009"},
         {date::year{2023} / 7 / 21, date::year{2023} / 7 / 24, "8006"},
         {date::year{2023} / 7 / 24, date::year{2023} / 7 / 25, "8005"},
         {date::year{2023} / 7 / 25, date::year{2023} / 7 / 26, "8003"},
         {date::year{2023} / 7 / 26, date::year{2023} / 7 / 27, "7999"},
         {date::year{2023} / 7 / 27, date::year{2023} / 7 / 28, "7997"},
         {date::year{2023} / 7 / 28, date::year{2023} / 7 / 31, "7994"},
         // Test old format
         {date::year{2023} / 7 / 31, date::year{2023} / 8 / 1, ""}}}}};
  return metadata;
}

Metadata GenInverseMetadata() {
  auto metadata = GenMetadata();
  metadata.stype_in = SType::InstrumentId;
  metadata.stype_out = SType::RawSymbol;
  std::vector<SymbolMapping> new_mappings;
  for (const auto& mapping : metadata.mappings) {
    for (const auto& interval : mapping.intervals) {
      if (interval.symbol.empty()) {
        continue;
      }
      new_mappings.push_back(
          SymbolMapping{interval.symbol,
                        {MappingInterval{interval.start_date, interval.end_date,
                                         mapping.raw_symbol}}});
    }
  }
  metadata.mappings = new_mappings;
  return metadata;
}

template <typename SM>
SM GenMapping(std::uint32_t instrument_id, const char* stype_out_symbol) {
  SM res = {sizeof(SM) / RecordHeader::kLengthMultiplier,
            RType::SymbolMapping,
            1,
            instrument_id,
            {}};
  std::strncpy(res.stype_out_symbol.data(), stype_out_symbol,
               res.stype_out_symbol.size());
  return res;
}

TEST(TsSymbolMapTests, TestBasic) {
  auto metadata = GenMetadata();
  TsSymbolMap target{metadata};
  EXPECT_EQ(target.At(date::year{2023} / 7 / 2, 32), "AAPL");
  EXPECT_EQ(target.At(date::year{2023} / 7 / 30, 32), "AAPL");
  EXPECT_EQ(target.At(date::year{2023} / 7 / 31, 32), "AAPL");
  EXPECT_EQ(target.Find(date::year{2023} / 8 / 1, 32), target.Map().end());
  EXPECT_EQ(target.At(date::year{2023} / 7 / 8, 8029), "PLTR");
  EXPECT_EQ(target.Find(date::year{2023} / 7 / 10, 8029), target.Map().end());
  EXPECT_EQ(target.At(date::year{2023} / 7 / 10, 8022), "PLTR");
  EXPECT_EQ(target.At(date::year{2023} / 7 / 20, 10184), "TSLA");
  EXPECT_EQ(target.At(date::year{2023} / 7 / 21, 10181), "TSLA");
  EXPECT_EQ(target.At(date::year{2023} / 7 / 24, 10174), "TSLA");
  EXPECT_EQ(target.At(date::year{2023} / 7 / 25, 10172), "TSLA");
  MboMsg record{
      RecordHeader{{},
                   RType::Mbo,
                   0,
                   10172,
                   UnixNanos{date::sys_days{date::year{2023} / 7 / 24}} +
                       std::chrono::hours{23}},
      {},
      {},
      {},
      {},
      {},
      {},
      {},
      UnixNanos{date::sys_days{date::year{2023} / 7 / 25}} + std::chrono::minutes{155},
      {},
      {}};
  EXPECT_EQ(target.At(record), "TSLA");
  auto it = target.Find(record);
  ASSERT_NE(it, target.Map().end());
  EXPECT_EQ(*it->second, "TSLA");
  EXPECT_EQ(target.At(date::year{2023} / 7 / 25, 10172), "TSLA");

  auto inverse_metadata = GenInverseMetadata();
  TsSymbolMap inverse_target{inverse_metadata};
  ASSERT_EQ(inverse_target.Size(), target.Size());
  for (const auto& kv : target.Map()) {
    EXPECT_EQ(*kv.second, inverse_target.At(kv.first.first, kv.first.second));
  }
}

TEST(TsSymbolMapTests, TestSTypeError) {
  auto metadata = GenMetadata();
  metadata.stype_out = SType::RawSymbol;
  ASSERT_THROW(TsSymbolMap{metadata}, InvalidArgumentError);
}

TEST(TsSymbolMapTests, TestInsertStartEndDateSame) {
  TsSymbolMap target;
  ASSERT_TRUE(target.Map().empty());
  target.Insert(1, date::year{2023} / 12 / 3, date::year{2023} / 12 / 3,
                std::make_shared<std::string>("test"));
  ASSERT_TRUE(target.Map().empty());
}

TEST(PitSymbolMapTests, TestFromMetadata) {
  auto metadata = GenMetadata();
  auto target = metadata.CreateSymbolMapForDate(date::year{2023} / 7 / 31);
  EXPECT_EQ(target.Size(), 4);
  EXPECT_EQ(target[32], "AAPL");
  EXPECT_EQ(target[7295], "NVDA");
  // NVDA from previous day
  EXPECT_EQ(target.Find(7298), target.Map().end());
  EXPECT_EQ(target[10163], "TSLA");
  EXPECT_EQ(target[6803], "MSFT");
  auto inverse_target = GenMetadata().CreateSymbolMapForDate(date::year{2023} / 7 / 31);
  EXPECT_EQ(inverse_target.Map(), target.Map());
}

TEST(PitSymbolMapTests, TestFromMetadataOutOfRange) {
  auto metadata = GenMetadata();
  ASSERT_EQ(metadata.start, UnixNanos{std::chrono::seconds{1688169600}});
  ASSERT_EQ(metadata.end, UnixNanos{std::chrono::seconds{1690848000}});
  ASSERT_THROW(PitSymbolMap(metadata, date::year{2023} / 8 / 1), InvalidArgumentError);
  ASSERT_THROW(PitSymbolMap(metadata, date::year{2023} / 6 / 30), InvalidArgumentError);
  metadata.end =
      UnixNanos{date::sys_days{date::year{2023} / 7 / 1}} + std::chrono::hours{8};
  ASSERT_NE(metadata.end, UnixNanos{date::sys_days{date::year{2023} / 7 / 1}});
  ASSERT_NO_THROW(PitSymbolMap(metadata, date::year{2023} / 7 / 1));
  ASSERT_THROW(PitSymbolMap(metadata, date::year{2023} / 7 / 2), InvalidArgumentError);
  metadata.end = UnixNanos{date::sys_days{date::year{2023} / 7 / 2}};
  ASSERT_THROW(PitSymbolMap(metadata, date::year{2023} / 7 / 2), InvalidArgumentError);
  metadata.end += std::chrono::nanoseconds{1};
  ASSERT_NO_THROW(PitSymbolMap(metadata, date::year{2023} / 7 / 2));
}

TEST(PitSymbolMapTests, TestOnSymbolMapping) {
  PitSymbolMap target;
  target.OnSymbolMapping(GenMapping<SymbolMappingMsgV1>(1, "AAPL"));
  target.OnSymbolMapping(GenMapping<SymbolMappingMsgV2>(2, "TSLA"));
  target.OnSymbolMapping(GenMapping<SymbolMappingMsgV1>(3, "MSFT"));
  std::unordered_map<std::uint32_t, std::string> exp{
      {1, "AAPL"}, {2, "TSLA"}, {3, "MSFT"}};
  ASSERT_EQ(target.Map(), exp);
  target.OnSymbolMapping(GenMapping<SymbolMappingMsgV1>(10, "AAPL"));
  target.OnSymbolMapping(GenMapping<SymbolMappingMsgV2>(1, "MSFT"));
  ASSERT_EQ(target[1], "MSFT");
}

TEST(PitSymbolMapTests, TestOnRecord) {
  PitSymbolMap target;
  auto sm1 = GenMapping<SymbolMappingMsgV1>(1, "AAPL");
  target.OnRecord(Record{&sm1.hd});
  auto sm2 = GenMapping<SymbolMappingMsgV2>(2, "TSLA");
  target.OnRecord(Record{&sm2.hd});
  sm1 = GenMapping<SymbolMappingMsgV1>(3, "MSFT");
  target.OnRecord(Record{&sm1.hd});
  std::unordered_map<std::uint32_t, std::string> exp{
      {1, "AAPL"}, {2, "TSLA"}, {3, "MSFT"}};
  ASSERT_EQ(target.Map(), exp);
  sm1 = GenMapping<SymbolMappingMsgV1>(10, "AAPL");
  target.OnRecord(Record{&sm1.hd});
  sm2 = GenMapping<SymbolMappingMsgV2>(1, "MSFT");
  target.OnRecord(Record{&sm2.hd});
  ASSERT_EQ(target[1], "MSFT");
}
}  // namespace databento::tests
