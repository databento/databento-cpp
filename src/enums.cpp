#include "databento/enums.hpp"

#include <cstdint>
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

const char* ToString(DurationInterval duration_interval) {
  switch (duration_interval) {
    case DurationInterval::Day: {
      return "day";
    }
    case DurationInterval::Week: {
      return "week";
    }
    case DurationInterval::Month: {
      return "month";
    }
    case DurationInterval::None: {
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

const char* ToString(BatchState state) {
  switch (state) {
    case BatchState::Received: {
      return "received";
    }
    case BatchState::Queued: {
      return "queued";
    }
    case BatchState::Processing: {
      return "processing";
    }
    case BatchState::Done: {
      return "done";
    }
    case BatchState::Expired: {
      return "expired";
    }
    default: {
      return "unknown";
    }
  }
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
                             "unknown value '" + str + "'"};
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
                             "unknown value '" + str + "'"};
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
                             "unknown value '" + str + "'"};
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
                             "unknown value '" + str + "'"};
}

template <>
DurationInterval FromString(const std::string& str) {
  if (str == "day") {
    return DurationInterval::Day;
  }
  if (str == "week") {
    return DurationInterval::Week;
  }
  if (str == "month") {
    return DurationInterval::Month;
  }
  if (str == "none") {
    return DurationInterval::None;
  }
  throw InvalidArgumentError{"FromString<DurationInterval>", "str",
                             "unknown value '" + str + "'"};
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
                             "unknown value '" + str + "'"};
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
                             "unknown value '" + str + "'"};
}

template <>
BatchState FromString(const std::string& str) {
  if (str == "received") {
    return BatchState::Received;
  }
  if (str == "queued") {
    return BatchState::Queued;
  }
  if (str == "processing") {
    return BatchState::Processing;
  }
  if (str == "done") {
    return BatchState::Done;
  }
  if (str == "expired") {
    return BatchState::Expired;
  }
  throw InvalidArgumentError{"FromString<BatchState>", "str",
                             "unknown value '" + str + "'"};
}
}  // namespace databento
