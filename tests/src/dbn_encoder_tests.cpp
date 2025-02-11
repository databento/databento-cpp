#include <date/date.h>
#include <gtest/gtest.h>

#include <chrono>
#include <memory>

#include "databento/constants.hpp"
#include "databento/datetime.hpp"
#include "databento/dbn.hpp"
#include "databento/dbn_decoder.hpp"
#include "databento/dbn_encoder.hpp"
#include "databento/log.hpp"
#include "mock/mock_io.hpp"

namespace databento::tests {
TEST(DbnEncoderTests, TestEncodeDecodeMetadataIdentity) {
  auto logger = std::make_unique<NullLogReceiver>();
  const Metadata metadata{
      kDbnVersion,
      dataset::kGlbxMdp3,
      false,
      Schema::Mbp10,
      UnixNanos{std::chrono::nanoseconds{1657230820000000000}},
      UnixNanos{std::chrono::nanoseconds{1658960170000000000}},
      0,
      false,
      SType::RawSymbol,
      SType::InstrumentId,
      true,
      kSymbolCstrLen,
      {"ES", "NG"},
      {"ESM2"},
      {"QQQQQ"},
      {{"ES.0",
        {{date::year{2022} / 7 / 26, date::year{2022} / 9 / 1, "ESU2"}}},
       {"NG.0",
        {{date::year{2022} / 7 / 26, date::year{2022} / 8 / 29, "NGU2"},
         {date::year{2022} / 8 / 29, date::year{2022} / 9 / 1, "NGV2"}}}}};
  mock::MockIo io{};
  DbnEncoder::EncodeMetadata(metadata, &io);
  DbnDecoder decoder{logger.get(),
                     std::make_unique<mock::MockIo>(std::move(io))};
  const auto res = decoder.DecodeMetadata();
  ASSERT_EQ(res, metadata);
}
}  // namespace databento::tests
