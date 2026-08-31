#pragma once

#include <date/date.h>
#include <nlohmann/json.hpp>

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "databento/datetime.hpp"            // UnixNanos
#include "databento/detail/http_client.hpp"  // HttpParams
#include "databento/enums.hpp"               // FromString
#include "databento/exceptions.hpp"          // JsonResponseError

namespace databento::detail {
void SetIfNotEmpty(HttpParams* params, const std::string& key,
                   const std::string& value);
void SetIfNotEmpty(HttpParams* params, const std::string& key,
                   const std::vector<databento::JobState>& states);

template <typename T>
void SetIfPositive(HttpParams* params, std::string_view key, const T value) {
  if (value > 0) {
    params->emplace(key, std::to_string(value));
  }
}

template <>
inline void SetIfPositive<databento::UnixNanos>(HttpParams* params,
                                                std::string_view key,
                                                const databento::UnixNanos value) {
  if (value.time_since_epoch().count()) {
    params->emplace(key, databento::ToString(value));
  }
}

const nlohmann::json& CheckedAt(std::string_view endpoint, const nlohmann::json& json,
                                std::string_view key);

template <typename T>
T FromCheckedAtString(std::string_view endpoint, const nlohmann::json& json,
                      std::string_view key) {
  const auto& val_json = CheckedAt(endpoint, json, key);
  if (!val_json.is_string()) {
    throw JsonResponseError::TypeMismatch(endpoint, std::string{key} + " string",
                                          val_json);
  }
  return databento::FromString<T>(val_json);
}

template <typename T>
T FromCheckedAtStringOrNull(std::string_view endpoint, const nlohmann::json& json,
                            std::string_view key, T null_value) {
  const auto& val_json = CheckedAt(endpoint, json, key);
  if (val_json.is_null()) {
    return null_value;
  }
  if (val_json.is_string()) {
    return databento::FromString<T>(val_json);
  }
  throw JsonResponseError::TypeMismatch(endpoint, std::string{key} + " null or string",
                                        val_json);
}

template <typename T>
T ParseAt(std::string_view endpoint, const nlohmann::json& json, std::string_view key);
template <>
bool ParseAt(std::string_view endpoint, const nlohmann::json& json,
             std::string_view key);
template <>
std::string ParseAt(std::string_view endpoint, const nlohmann::json& json,
                    std::string_view key);
template <>
std::optional<std::string> ParseAt(std::string_view endpoint,
                                   const nlohmann::json& json, std::string_view key);
template <>
std::uint64_t ParseAt(std::string_view endpoint, const nlohmann::json& json,
                      std::string_view key);
template <>
std::uint16_t ParseAt(std::string_view endpoint, const nlohmann::json& json,
                      std::string_view key);
template <>
double ParseAt(std::string_view endpoint, const nlohmann::json& json,
               std::string_view key);
template <>
std::vector<std::string> ParseAt(std::string_view endpoint, const nlohmann::json& json,
                                 std::string_view key);
template <>
date::year_month_day ParseAt(std::string_view endpoint, const nlohmann::json& json,
                             std::string_view key);
template <>
std::optional<std::uint8_t> ParseAt(std::string_view endpoint,
                                    const nlohmann::json& json, std::string_view key);

}  // namespace databento::detail
