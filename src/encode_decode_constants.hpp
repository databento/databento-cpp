#pragma once

#include <cstdint>
#include <limits>

constexpr std::size_t kMagicSize = 4;
constexpr std::uint32_t kZstdMagicNumber = 0xFD2FB528;
constexpr auto kDbnPrefix = "DBN";
constexpr std::size_t kFixedMetadataLen = 100;
constexpr std::size_t kDatasetCstrLen = 16;
constexpr std::size_t kReservedLen = 53;
constexpr std::size_t kReservedLenV1 = 47;
constexpr std::size_t kBufferCapacity = 8UL * 1024;



constexpr std::uint32_t kSymbolCstrLenV1 = 22;
constexpr std::uint64_t NULL_RECORD_COUNT = std::numeric_limits<std::uint64_t>::max();
constexpr std::uint64_t NULL_LIMIT = 0;
constexpr std::uint16_t NULL_SCHEMA = std::numeric_limits<std::uint16_t>::max();
constexpr std::uint8_t NULL_STYPE = std::numeric_limits<std::uint8_t>::max();

