#include "databento/enums.hpp"

#include <cstdint>
#include <ostream>
#include <string>

#include "databento/exceptions.hpp"  // InvalidArgumentError

namespace databento {
const char* UrlFromGateway(HistoricalGateway gateway) {
  switch (gateway) {
    case HistoricalGateway::Nearest:
    case HistoricalGateway::Bo1: {
      return "https://hist.databento.com";
    }
    default: {
      throw InvalidArgumentError{
          "UrlFromGateway", "gateway",
          "unknown value " +
              std::to_string(static_cast<std::uint8_t>(gateway))};
    }
  }
}

const char* ToString(Schema schema) {
  switch (schema) {
    case Schema::Mbo: {
      return "mbo";
    }
    case Schema::Mbp1: {
      return "mbp-1";
    }
    case Schema::Mbp10: {
      return "mbp-10";
    }
    case Schema::Tbbo: {
      return "tbbo";
    }
    case Schema::Trades: {
      return "trades";
    }
    case Schema::Ohlcv1S: {
      return "ohlcv-1s";
    }
    case Schema::Ohlcv1M: {
      return "ohlcv-1m";
    }
    case Schema::Ohlcv1H: {
      return "ohlcv-1h";
    }
    case Schema::Ohlcv1D: {
      return "ohlcv-1d";
    }
    case Schema::Definition: {
      return "definition";
    }
    case Schema::Statistics: {
      return "statistics";
    }
    case Schema::Status: {
      return "status";
    }
    default: {
      return "unknown";
    }
  }
}

const char* ToString(Encoding encoding) {
  switch (encoding) {
    case Encoding::Dbz: {
      return "dbz";
    }
    case Encoding::Csv: {
      return "csv";
    }
    case Encoding::Json: {
      return "json";
    }
    default: {
      return "unknown";
    }
  }
}

const char* ToString(FeedMode mode) {
  switch (mode) {
    case FeedMode::Historical: {
      return "historical";
    }
    case FeedMode::HistoricalStreaming: {
      return "historical-streaming";
    }
    case FeedMode::Live: {
      return "live";
    }
    default: {
      return "unknown";
    }
  }
}

const char* ToString(Compression compression) {
  switch (compression) {
    case Compression::None: {
      return "none";
    }
    case Compression::Zstd: {
      return "zstd";
    }
    default: {
      return "unknown";
    }
  }
}

const char* ToString(SType stype) {
  switch (stype) {
    case SType::ProductId: {
      return "product_id";
    }
    case SType::Native: {
      return "native";
    }
    case SType::Smart: {
      return "smart";
    }
    default: {
      return "unknown";
    }
  }
}

const char* ToString(SplitDuration duration_interval) {
  switch (duration_interval) {
    case SplitDuration::Day: {
      return "day";
    }
    case SplitDuration::Week: {
      return "week";
    }
    case SplitDuration::Month: {
      return "month";
    }
    case SplitDuration::None: {
      return "none";
    }
    default: {
      return "unknown";
    }
  }
}

const char* ToString(Packaging packaging) {
  switch (packaging) {
    case Packaging::None: {
      return "none";
    }
    case Packaging::Zip: {
      return "zip";
    }
    case Packaging::Tar: {
      return "tar";
    }
    default: {
      return "unknown";
    }
  }
}

const char* ToString(Delivery delivery) {
  switch (delivery) {
    case Delivery::Download: {
      return "download";
    }
    case Delivery::S3: {
      return "s3";
    }
    case Delivery::Disk: {
      return "disk";
    }
    default: {
      return "unknown";
    }
  }
}

const char* ToString(JobState state) {
  switch (state) {
    case JobState::Received: {
      return "received";
    }
    case JobState::Queued: {
      return "queued";
    }
    case JobState::Processing: {
      return "processing";
    }
    case JobState::Done: {
      return "done";
    }
    case JobState::Expired: {
      return "expired";
    }
    default: {
      return "unknown";
    }
  }
}

const char* ToString(DatasetCondition condition) {
  switch (condition) {
    case DatasetCondition::Available: {
      return "available";
    }
    case DatasetCondition::Bad: {
      return "bad";
    }
    default: {
      return "unknown";
    }
  }
}

std::ostream& operator<<(std::ostream& out, Schema schema) {
  out << ToString(schema);
  return out;
}

std::ostream& operator<<(std::ostream& out, Encoding encoding) {
  out << ToString(encoding);
  return out;
}

std::ostream& operator<<(std::ostream& out, FeedMode mode) {
  out << ToString(mode);
  return out;
}

std::ostream& operator<<(std::ostream& out, Compression compression) {
  out << ToString(compression);
  return out;
}

std::ostream& operator<<(std::ostream& out, SType stype) {
  out << ToString(stype);
  return out;
}

std::ostream& operator<<(std::ostream& out, SplitDuration duration_interval) {
  out << ToString(duration_interval);
  return out;
}

std::ostream& operator<<(std::ostream& out, Packaging packaging) {
  out << ToString(packaging);
  return out;
}

std::ostream& operator<<(std::ostream& out, Delivery delivery) {
  out << ToString(delivery);
  return out;
}

std::ostream& operator<<(std::ostream& out, JobState state) {
  out << ToString(state);
  return out;
}

std::ostream& operator<<(std::ostream& out, DatasetCondition condition) {
  out << ToString(condition);
  return out;
}

template <>
Schema FromString(const std::string& str) {
  if (str == "mbo") {
    return Schema::Mbo;
  }
  if (str == "mbp-1") {
    return Schema::Mbp1;
  }
  if (str == "mbp-10") {
    return Schema::Mbp10;
  }
  if (str == "tbbo") {
    return Schema::Tbbo;
  }
  if (str == "trades") {
    return Schema::Trades;
  }
  if (str == "ohlcv-1s") {
    return Schema::Ohlcv1S;
  }
  if (str == "ohlcv-1m") {
    return Schema::Ohlcv1M;
  }
  if (str == "ohlcv-1h") {
    return Schema::Ohlcv1H;
  }
  if (str == "ohlcv-1d") {
    return Schema::Ohlcv1D;
  }
  if (str == "definition") {
    return Schema::Definition;
  }
  if (str == "statistics") {
    return Schema::Statistics;
  }
  if (str == "status") {
    return Schema::Status;
  }
  throw InvalidArgumentError{"FromString<Schema>", "str",
                             "unknown value '" + str + '\''};
}

template <>
Encoding FromString(const std::string& str) {
  if (str == "dbz") {
    return Encoding::Dbz;
  }
  if (str == "csv") {
    return Encoding::Csv;
  }
  if (str == "json") {
    return Encoding::Json;
  }
  throw InvalidArgumentError{"FromString<Encoding>", "str",
                             "unknown value '" + str + '\''};
}

template <>
FeedMode FromString(const std::string& str) {
  if (str == "historical") {
    return FeedMode::Historical;
  }
  if (str == "historical-streaming") {
    return FeedMode::HistoricalStreaming;
  }
  if (str == "live") {
    return FeedMode::Live;
  }
  throw InvalidArgumentError{"FromString<FeedMode>", "str",
                             "unknown value '" + str + '\''};
}

template <>
Compression FromString(const std::string& str) {
  if (str == "none") {
    return Compression::None;
  }
  if (str == "zstd") {
    return Compression::Zstd;
  }
  throw InvalidArgumentError{"FromString<Compression>", "str",
                             "unknown value '" + str + '\''};
}

template <>
SType FromString(const std::string& str) {
  if (str == "product_id") {
    return SType::ProductId;
  }
  if (str == "native") {
    return SType::Native;
  }
  if (str == "smart") {
    return SType::Smart;
  }
  throw InvalidArgumentError{"FromString<SType>", "str",
                             "unknown value '" + str + '\''};
}

template <>
SplitDuration FromString(const std::string& str) {
  if (str == "day") {
    return SplitDuration::Day;
  }
  if (str == "week") {
    return SplitDuration::Week;
  }
  if (str == "month") {
    return SplitDuration::Month;
  }
  if (str == "none") {
    return SplitDuration::None;
  }
  throw InvalidArgumentError{"FromString<SplitInterval>", "str",
                             "unknown value '" + str + '\''};
}

template <>
Packaging FromString(const std::string& str) {
  if (str == "none") {
    return Packaging::None;
  }
  if (str == "zip") {
    return Packaging::Zip;
  }
  if (str == "tar") {
    return Packaging::Tar;
  }
  throw InvalidArgumentError{"FromString<Packaging>", "str",
                             "unknown value '" + str + '\''};
}

template <>
Delivery FromString(const std::string& str) {
  if (str == "download") {
    return Delivery::Download;
  }
  if (str == "s3") {
    return Delivery::S3;
  }
  if (str == "disk") {
    return Delivery::Disk;
  }
  throw InvalidArgumentError{"FromString<Delivery>", "str",
                             "unknown value '" + str + '\''};
}

template <>
JobState FromString(const std::string& str) {
  if (str == "received") {
    return JobState::Received;
  }
  if (str == "queued") {
    return JobState::Queued;
  }
  if (str == "processing") {
    return JobState::Processing;
  }
  if (str == "done") {
    return JobState::Done;
  }
  if (str == "expired") {
    return JobState::Expired;
  }
  throw InvalidArgumentError{"FromString<JobState>", "str",
                             "unknown value '" + str + '\''};
}

template <>
DatasetCondition FromString(const std::string& str) {
  if (str == "available") {
    return DatasetCondition::Available;
  }
  if (str == "bad") {
    return DatasetCondition::Bad;
  }
  throw InvalidArgumentError{"FromString<DatasetCondition>", "str",
                             "unknown value '" + str + '\''};
}
}  // namespace databento
