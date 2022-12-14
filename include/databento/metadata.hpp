#pragma once

#include <map>
#include <ostream>
#include <string>
#include <vector>

#include "databento/enums.hpp"  // FeedMode, DatasetCondition, Schema

namespace databento {
// mapping of field name to type name
using FieldDefinition = std::map<std::string, std::string>;
using FieldsByEncodingAndSchema =
    std::map<Encoding, std::map<Schema, FieldDefinition>>;
using FieldsByDatasetEncodingAndSchema =
    std::map<std::string, FieldsByEncodingAndSchema>;
using PriceBySchema = std::map<Schema, double>;
using PriceByFeedModeAndSchema = std::map<FeedMode, PriceBySchema>;
using PriceByFeedMode = std::map<FeedMode, double>;

struct DatasetConditionDetail {
  std::string date;
  DatasetCondition condition;
};

struct DatasetConditionInfo {
  // Overall condition for the date range.
  DatasetCondition condition;
  // The dataset condition by date.
  std::vector<DatasetConditionDetail> details;
  std::string adjusted_start_date;
  std::string adjusted_end_date;
};

std::string ToString(const DatasetConditionDetail& condition_detail);
std::ostream& operator<<(std::ostream& stream,
                         const DatasetConditionDetail& condition_detail);
std::string ToString(const DatasetConditionInfo& condition);
std::ostream& operator<<(std::ostream& stream,
                         const DatasetConditionInfo& condition);
}  // namespace databento
