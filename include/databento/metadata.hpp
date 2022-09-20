#pragma once

#include <map>

#include "databento/enums.hpp"

namespace databento {
using PriceByFeedModeAndSchema = std::map<FeedMode, std::map<Schema, double>>;
using PriceByFeedMode = std::map<FeedMode, double>;
using PriceBySchema = std::map<Schema, double>;
}  // namespace databento