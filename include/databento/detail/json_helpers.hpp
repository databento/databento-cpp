#pragma once

#include <date/date.h>
#include <nlohmann/json.hpp>

#include <map>  // multimap
#include <string>
#include <vector>

#include "databento/datetime.hpp"    // UnixNanos
#include "databento/enums.hpp"       // FromString
#include "databento/exceptions.hpp"  // JsonResponseError

namespace httplib {
using Params = std::multimap<std::string, std::string>;
}

namespace databento::detail {
void SetIfNotEmpty(httplib::Params* params, const std::string& key,
                   const std::string& value);
void SetIfNotEmpty(httplib::Params* params, const std::string& key,
                   const std::vector<databento::JobState>& states);

template <typename T>
void SetIfPositive(httplib::Params* params, const std::string& key,
                   const T value) {
  if (value > 0) {
    params->emplace(key, std::to_string(value));
  }
}

template <>
inline void SetIfPositive<databento::UnixNanos>(
    httplib::Params* params, const std::string& key,
    const databento::UnixNanos value) {
  if (value.time_since_epoch().count()) {
    params->emplace(key, databento::ToString(value));
  }
}

const nlohmann::json& CheckedAt(const std::string& endpoint,
                                const nlohmann::json& json,
                                const std::string& key);

template <typename T>
T FromCheckedAtString(const std::string& endpoint, const nlohmann::json& json,
                      const std::string& key) {
  const auto& val_json = CheckedAt(endpoint, json, key);
  if (!val_json.is_string()) {
    throw JsonResponseError::TypeMismatch(endpoint, key + " string", val_json);
  }
  return databento::FromString<T>(val_json);
}

template <typename T>
T FromCheckedAtStringOrNull(const std::string& endpoint,
                            const nlohmann::json& json, const std::string& key,
                            T null_value) {
  const auto& val_json = CheckedAt(endpoint, json, key);
  if (val_json.is_null()) {
    return null_value;
  }
  if (val_json.is_string()) {
    return databento::FromString<T>(val_json);
  }
  throw JsonResponseError::TypeMismatch(endpoint, key + " null or string",
                                        val_json);
}

template <typename T>
T ParseAt(const std::string& endpoint, const nlohmann::json& json,
          const std::string& key);
template <>
bool ParseAt(const std::string& endpoint, const nlohmann::json& json,
             const std::string& key);
template <>
std::string ParseAt(const std::string& endpoint, const nlohmann::json& json,
                    const std::string& key);
template <>
std::uint64_t ParseAt(const std::string& endpoint, const nlohmann::json& json,
                      const std::string& key);
template <>
std::uint16_t ParseAt(const std::string& endpoint, const nlohmann::json& json,
                      const std::string& key);
template <>
double ParseAt(const std::string& endpoint, const nlohmann::json& json,
               const std::string& key);
template <>
std::vector<std::string> ParseAt(const std::string& endpoint,
                                 const nlohmann::json& json,
                                 const std::string& key);
template <>
date::year_month_day ParseAt(const std::string& endpoint,
                             const nlohmann::json& json,
                             const std::string& key);

}  // namespace databento::detail
