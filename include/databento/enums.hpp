#pragma once

#include <cstdint>
#include <iosfwd>
#include <string>

namespace databento {
// Represents a historical data center gateway location.
enum class HistoricalGateway : std::uint8_t {
  Bo1,
};

// Represents a live data center gateway location.
enum class LiveGateway : std::uint8_t {
  Origin = 0,
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

// Represents a data output encoding.
enum class Encoding : std::uint8_t {
  Dbn = 0,
  Csv = 1,
  Json = 2,
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

// Represents the duration of time at which batch files will be split.
enum class SplitDuration : std::uint8_t {
  Day = 0,
  Week,
  Month,
  None,
};

// Represents how a batch job will be packaged.
enum class Packaging : std::uint8_t {
  None = 0,
  Zip,
  Tar,
};

// Represents how a batch job will be delivered.
enum class Delivery : std::uint8_t {
  Download,
  S3,
  Disk,
};

// The current state of a batch job.
enum class JobState : std::uint8_t {
  Received,
  Queued,
  Processing,
  Done,
  Expired,
};

// The condition of a dataset at a point in time.
enum class DatasetCondition : std::uint8_t {
  Available,
  Bad,
};

// Sentinel values for different DBN record types.
//
// Additional rtypes may be added in the future.
namespace rtype {
enum RType : std::uint8_t {
  Mbp0 = 0x00,
  Mbp1 = 0x01,
  Mbp10 = 0x0A,
  Ohlcv = 0x11,
  InstrumentDef = 0x13,
  Error = 0x15,
  SymbolMapping = 0x16,
  Mbo = 0xA0,
};
}  // namespace rtype
using rtype::RType;

// A tick action.
//
// Additional actions may be added in the future.
namespace action {
enum Action : char {
  // An existing order was modified.
  Modify = 'M',
  // A trade executed.
  Trade = 'T',
  // An order was cancelled.
  Cancel = 'C',
  // A new order was added.
  Add = 'A',
  // Reset the book; clear all orders for an instrument.
  Clear = 'R',
};
}  // namespace action
using action::Action;

// A side of the market. The side of the market for resting orders, or the side
// of the aggressor for trades.
enum class Side : char {
  // A sell order.
  Ask = 'A',
  // A buy order.
  Bid = 'B',
  // None or unknown.
  None = 'N',
};

// Convert a HistoricalGateway to a URL.
const char* UrlFromGateway(HistoricalGateway gateway);

const char* ToString(Schema schema);
const char* ToString(Encoding encoding);
const char* ToString(FeedMode mode);
const char* ToString(Compression compression);
const char* ToString(SType stype);
const char* ToString(SplitDuration duration_interval);
const char* ToString(Packaging packaging);
const char* ToString(Delivery delivery);
const char* ToString(JobState state);
const char* ToString(DatasetCondition condition);
const char* ToString(RType rtype);
const char* ToString(Action action);
const char* ToString(Side side);

std::ostream& operator<<(std::ostream& out, Schema schema);
std::ostream& operator<<(std::ostream& out, Encoding encoding);
std::ostream& operator<<(std::ostream& out, FeedMode mode);
std::ostream& operator<<(std::ostream& out, Compression compression);
std::ostream& operator<<(std::ostream& out, SType stype);
std::ostream& operator<<(std::ostream& out, SplitDuration duration_interval);
std::ostream& operator<<(std::ostream& out, Packaging packaging);
std::ostream& operator<<(std::ostream& out, Delivery delivery);
std::ostream& operator<<(std::ostream& out, JobState state);
std::ostream& operator<<(std::ostream& out, DatasetCondition condition);
std::ostream& operator<<(std::ostream& out, RType rtype);
std::ostream& operator<<(std::ostream& out, Action action);
std::ostream& operator<<(std::ostream& out, Side side);

template <typename T>
T FromString(const std::string& str);
template <>
Schema FromString(const std::string& str);
template <>
Encoding FromString(const std::string& str);
template <>
FeedMode FromString(const std::string& str);
template <>
Compression FromString(const std::string& str);
template <>
SType FromString(const std::string& str);
template <>
SplitDuration FromString(const std::string& str);
template <>
Packaging FromString(const std::string& str);
template <>
Delivery FromString(const std::string& str);
template <>
JobState FromString(const std::string& str);
template <>
DatasetCondition FromString(const std::string& str);
}  // namespace databento
