#pragma once

#include <map>
#include <string>

#include "databento/enums.hpp"  // FeedMode, Schema

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
}  // namespace databento
