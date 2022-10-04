#pragma once

#include <cstdint>
#include <string>

namespace databento {
// Represents a historical data center gateway location.
enum class HistoricalGateway : std::uint8_t {
  Nearest = 0,
  Bo1,
};

// Represents a live data center gateway location.
enum class LiveGateway : std::uint8_t {
  Origin = 0,
  Nearest,
  Ny4,
  Dc3,
};

// Represents a data feed mode.
enum class FeedMode : std::uint8_t {
  Historical,
  HistoricalStreaming,
  Live,
};

// Represents a data record schema.
enum class Schema : std::uint16_t {
  Mbo = 0,
  Mbp1 = 1,
  Mbp10 = 2,
  Tbbo = 3,
  Trades = 4,
  Ohlcv1S = 5,
  Ohlcv1M = 6,
  Ohlcv1H = 7,
  Ohlcv1D = 8,
  Definition = 9,
  Statistics = 10,
  Status = 11,
};

// Represents a data compression format (if any).
enum class Compression : std::uint8_t {
  None = 0,
  Zstd = 1,
};

// Represents a symbology type.
enum class SType : std::uint8_t {
  ProductId = 0,
  Native = 1,
  Smart = 2,
};

enum class DurationInterval : std::uint8_t {
  Day = 0,
  Week,
  Month,
  None,
};

enum class Packaging : std::uint8_t {
  None = 0,
  Zip,
  Tar,
};

enum class Delivery : std::uint8_t {
  Download,
  S3,
  Disk,
};

enum class Flag : std::uint8_t {
  kLast = 1 << 7,
  kHalt = 1 << 6,
  kReset = 1 << 5,
  kDupId = 1 << 4,
  kMbp = 1 << 3,
};

enum class BatchState : std::uint8_t {
  Received,
  Queued,
  Processing,
  Done,
  Expired,
};

const char* UrlFromGateway(HistoricalGateway gateway);

const char* ToString(Schema schema);
const char* ToString(FeedMode mode);
const char* ToString(Compression compression);
const char* ToString(SType stype);
const char* ToString(DurationInterval duration_interval);
const char* ToString(Packaging packaging);
const char* ToString(Delivery delivery);
const char* ToString(BatchState state);

template <typename T>
T FromString(const std::string& str);
template <>
Schema FromString(const std::string& str);
template <>
FeedMode FromString(const std::string& str);
template <>
Compression FromString(const std::string& str);
template <>
SType FromString(const std::string& str);
template <>
DurationInterval FromString(const std::string& str);
template <>
Packaging FromString(const std::string& str);
template <>
Delivery FromString(const std::string& str);
template <>
BatchState FromString(const std::string& str);
}  // namespace databento
