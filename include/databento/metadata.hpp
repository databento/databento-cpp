#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <ostream>
#include <string>
#include <unordered_map>

#include "databento/datetime.hpp"
#include "databento/enums.hpp"  // FeedMode, DatasetCondition, Schema

namespace databento {
struct PublisherDetail {
  std::uint16_t publisher_id;
  std::string dataset;
  std::string venue;
  std::string description;
};

struct FieldDetail {
  std::string name;
  std::string type;
};

struct UnitPricesForMode {
  FeedMode mode;
  std::map<Schema, double> unit_prices;
};

struct DatasetConditionDetail {
  std::string date;
  DatasetCondition condition;
  std::optional<std::string> last_modified_date;
};

struct DatasetRange {
  std::string start;
  std::string end;
  std::map<Schema, DateTimeRange<std::string>> range_by_schema;
};

inline bool operator==(const PublisherDetail& lhs, const PublisherDetail& rhs) {
  return lhs.publisher_id == rhs.publisher_id && lhs.dataset == rhs.dataset &&
         lhs.venue == rhs.venue && lhs.description == rhs.description;
}
inline bool operator!=(const PublisherDetail& lhs, const PublisherDetail& rhs) {
  return !(lhs == rhs);
}

inline bool operator==(const FieldDetail& lhs, const FieldDetail& rhs) {
  return lhs.name == rhs.name && lhs.type == rhs.type;
}
inline bool operator!=(const FieldDetail& lhs, const FieldDetail& rhs) {
  return !(lhs == rhs);
}

inline bool operator==(const UnitPricesForMode& lhs, const UnitPricesForMode& rhs) {
  return lhs.mode == rhs.mode && lhs.unit_prices == rhs.unit_prices;
}
inline bool operator!=(const UnitPricesForMode& lhs, const UnitPricesForMode& rhs) {
  return !(lhs == rhs);
}

inline bool operator==(const DatasetConditionDetail& lhs,
                       const DatasetConditionDetail& rhs) {
  return lhs.date == rhs.date && lhs.condition == rhs.condition &&
         lhs.last_modified_date == rhs.last_modified_date;
}
inline bool operator!=(const DatasetConditionDetail& lhs,
                       const DatasetConditionDetail& rhs) {
  return !(lhs == rhs);
}

inline bool operator==(const DatasetRange& lhs, const DatasetRange& rhs) {
  return lhs.start == rhs.start && lhs.end == rhs.end &&
         lhs.range_by_schema == rhs.range_by_schema;
}
inline bool operator!=(const DatasetRange& lhs, const DatasetRange& rhs) {
  return !(lhs == rhs);
}

std::string ToString(const PublisherDetail& publisher_detail);
std::ostream& operator<<(std::ostream& stream, const PublisherDetail& publisher_detail);
std::string ToString(const FieldDetail& field_detail);
std::ostream& operator<<(std::ostream& stream, const FieldDetail& field_detail);
std::string ToString(const DatasetConditionDetail& condition_detail);
std::ostream& operator<<(std::ostream& stream,
                         const DatasetConditionDetail& condition_detail);
std::string ToString(const DatasetRange& dataset_range);
std::ostream& operator<<(std::ostream& stream, const DatasetRange& dataset_range);
}  // namespace databento
