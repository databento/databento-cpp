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
}  // namespace databento::tests
