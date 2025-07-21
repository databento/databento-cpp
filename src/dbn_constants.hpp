#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>

namespace databento {
constexpr std::size_t kMagicSize = 4;
constexpr std::size_t kMetadataPreludeSize = 8;
constexpr std::uint32_t kZstdMagicNumber = 0xFD2FB528;
constexpr auto kDbnPrefix = "DBN";
constexpr std::size_t kFixedMetadataLen = 100;
constexpr std::size_t kDatasetCstrLen = 16;
constexpr std::size_t kMetadataReservedLen = 53;
constexpr std::size_t kMetadataReservedLenV1 = 47;
constexpr std::uint16_t kNullSchema = std::numeric_limits<std::uint16_t>::max();
constexpr std::uint8_t kNullSType = std::numeric_limits<std::uint8_t>::max();
constexpr std::uint64_t kNullRecordCount = std::numeric_limits<std::uint64_t>::max();
}  // namespace databento
