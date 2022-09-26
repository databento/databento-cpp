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
  Mbo,
  Mbp1,
  Mbp10,
  Tbbo,
  Trades,
  Ohlcv1S,
  Ohlcv1M,
  Ohlcv1H,
  Ohlcv1D,
  Definition,
  Statistics,
  Status,
};

// Represents a data compression format (if any).
enum class Compression : std::uint8_t {
  None,
  Zstd,
};

// Represents a symbology type.
enum class SType : std::uint8_t {
  ProductId,
  Native,
  Smart,
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
