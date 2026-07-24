#include <date/date.h>
#include <gtest/gtest.h>

#include <chrono>
#include <memory>

#include "databento/constants.hpp"
#include "databento/datetime.hpp"
#include "databento/dbn.hpp"
#include "databento/dbn_decoder.hpp"
#include "databento/dbn_encoder.hpp"
#include "databento/detail/buffer.hpp"
#include "databento/exceptions.hpp"
#include "databento/log.hpp"
#include "databento/record.hpp"
#include "databento/with_ts_out.hpp"
#include "mock/mock_log_receiver.hpp"

namespace databento::tests {
TEST(DbnEncoderTests, TestEncodeDecodeMetadataIdentity) {
  auto logger = mock::MockLogReceiver::AssertNoLogs(LogLevel::Warning);
  const Metadata metadata{
      kDbnVersion,
      dataset::kGlbxMdp3,
      Schema::Mbp10,
      UnixNanos{std::chrono::nanoseconds{1657230820000000000}},
      UnixNanos{std::chrono::nanoseconds{1658960170000000000}},
      0,
      SType::RawSymbol,
      SType::InstrumentId,
      true,
      kSymbolCstrLen,
      {"ES", "NG"},
      {"ESM2"},
      {"QQQQQ"},
      {{"ES.0", {{date::year{2022} / 7 / 26, date::year{2022} / 9 / 1, "ESU2"}}},
       {"NG.0",
        {{date::year{2022} / 7 / 26, date::year{2022} / 8 / 29, "NGU2"},
         {date::year{2022} / 8 / 29, date::year{2022} / 9 / 1, "NGV2"}}}}};
  detail::Buffer io{};
  DbnEncoder::EncodeMetadata(metadata, &io);
  DbnDecoder decoder{&logger, std::make_unique<detail::Buffer>(std::move(io))};
  const auto res = decoder.DecodeMetadata();
  ASSERT_EQ(res, metadata);
}

TEST(DbnEncoderTests, TestEncodeNewerMetadataErrors) {
  const Metadata metadata{kDbnVersion + 1,
                          dataset::kGlbxMdp3,
                          Schema::Mbp10,
                          {},
                          UnixNanos{},
                          0,
                          SType::RawSymbol,
                          SType::InstrumentId,
                          true,
                          kSymbolCstrLen,
                          {},
                          {},
                          {},
                          {}};
  detail::Buffer io{};
  ASSERT_THROW(DbnEncoder::EncodeMetadata(metadata, &io),
               databento::InvalidArgumentError);
}

TEST(DbnEncoderTests, TestEncodeRecordWithTsOutPreservesTsOut) {
  auto logger = mock::MockLogReceiver::AssertNoLogs(LogLevel::Warning);
  const Metadata metadata{kDbnVersion,
                          dataset::kGlbxMdp3,
                          Schema::Mbp1,
                          UnixNanos{},
                          UnixNanos{},
                          0,
                          SType::RawSymbol,
                          SType::InstrumentId,
                          true,  // ts_out
                          kSymbolCstrLen,
                          {},
                          {},
                          {},
                          {}};
  const UnixNanos ts_out{std::chrono::nanoseconds{1658441851999000000}};
  const WithTsOut<Mbp1Msg> orig{
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
      ts_out};

  detail::Buffer io{};
  DbnEncoder encoder{metadata, &io};
  // Encoding must use the runtime record size from the header `length`ts_out` survives
  // the round-trip
  encoder.EncodeRecord(orig);

  DbnDecoder decoder{&logger, std::make_unique<detail::Buffer>(std::move(io))};
  decoder.DecodeMetadata();
  const auto* record = decoder.DecodeRecord();
  ASSERT_NE(record, nullptr);
  EXPECT_EQ(record->Size(), sizeof(Mbp1Msg) + sizeof(UnixNanos));
  const auto& decoded = record->Get<WithTsOut<Mbp1Msg>>();
  EXPECT_EQ(decoded.rec, orig.rec);
  EXPECT_EQ(decoded.ts_out, ts_out);
}
}  // namespace databento::tests
