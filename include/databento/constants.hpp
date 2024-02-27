#pragma once

#include <cstdint>
#include <limits>

namespace databento {
static constexpr auto kApiVersion = 0;
static constexpr auto kApiVersionStr = "0";
static constexpr auto kApiKeyLength = 32;
// The decimal scaler of fixed prices.
static constexpr std::int64_t kFixedPriceScale = 1000000000;
// The sentinel value for an unset or null price.
static constexpr auto kUndefPrice = std::numeric_limits<std::int64_t>::max();
// The sentinel value for an unset or null order size.
static constexpr auto kUndefOrderSize =
    std::numeric_limits<std::uint32_t>::max();
// The sentinel value for an unset statistic quantity.
static constexpr auto kUndefStatQuantity =
    std::numeric_limits<std::int32_t>::max();
// The sentinel value for an unset or null timestamp.
static constexpr auto kUndefTimestamp =
    std::numeric_limits<std::uint64_t>::max();
// The current version of the DBN encoding.
static constexpr auto kDbnVersion = 2;
// The length of fixed-length symbol strings.
static constexpr auto kSymbolCstrLen = 71;
// The multiplier for converting the `length` field in `RecordHeader` to bytes.
static constexpr std::size_t kRecordHeaderLengthMultiplier = 4;

// This is not necessarily a comprehensive list of available datasets. Please
// use `Historical.MetadataListDatasets` to retrieve an up-to-date list.
namespace dataset {
// The dataset code for Databento Equities Basic.
static constexpr auto kDbeqBasic = "DBEQ.BASIC";
// The dataset code for CME Globex MDP 3.0.
static constexpr auto kGlbxMdp3 = "GLBX.MDP3";
// The dataset code for OPRA.PILLAR.
static constexpr auto kOpraPillar = "OPRA.PILLAR";
// The dataset code for Nasdaq TotalView-ITCH.
static constexpr auto kXnasItch = "XNAS.ITCH";
}  // namespace dataset
}  // namespace databento
