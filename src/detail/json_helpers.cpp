#include "databento/detail/json_helpers.hpp"

#include <numeric>  // accumulate
#include <sstream>  // istringstream

namespace databento {
namespace detail {
void SetIfNotEmpty(httplib::Params* params, const std::string& key,
                   const std::string& value) {
  if (!value.empty()) {
    params->emplace(key, value);
  }
}

void SetIfNotEmpty(httplib::Params* params, const std::string& key,
                   const std::vector<databento::JobState>& states) {
  if (!states.empty()) {
    std::string value = std::accumulate(
        states.begin(), states.end(), std::string{},
        [](std::string acc, databento::JobState state) {
          return acc.empty()
                     ? databento::ToString(state)
                     : std::move(acc) + "," + databento::ToString(state);
        });
    params->emplace(key, std::move(value));
  }
}

const nlohmann::json& CheckedAt(const std::string& endpoint,
                                const nlohmann::json& json,
                                const std::string& key) {
  if (json.contains(key)) {
    return json.at(key);
  }
  throw JsonResponseError::MissingKey(endpoint, key);
}

template <>
bool ParseAt(const std::string& endpoint, const nlohmann::json& json,
             const std::string& key) {
  const auto& val_json = CheckedAt(endpoint, json, key);
  if (!val_json.is_boolean()) {
    throw JsonResponseError::TypeMismatch(endpoint, key + " bool", val_json);
  }
  return val_json;
}

template <>
std::string ParseAt(const std::string& endpoint, const nlohmann::json& json,
                    const std::string& key) {
  const auto& val_json = CheckedAt(endpoint, json, key);
  if (val_json.is_null()) {
    return {};
  }
  if (!val_json.is_string()) {
    throw JsonResponseError::TypeMismatch(endpoint, key + " string", val_json);
  }
  return val_json;
}

template <>
std::uint64_t ParseAt(const std::string& endpoint, const nlohmann::json& json,
                      const std::string& key) {
  const auto& val_json = CheckedAt(endpoint, json, key);
  if (val_json.is_null()) {
    return 0;
  }
  if (!val_json.is_number_unsigned()) {
    throw JsonResponseError::TypeMismatch(endpoint, key + " unsigned number",
                                          val_json);
  }
  return val_json;
}

template <>
std::uint16_t ParseAt(const std::string& endpoint, const nlohmann::json& json,
                      const std::string& key) {
  const auto& val_json = CheckedAt(endpoint, json, key);
  if (val_json.is_null()) {
    return 0;
  }
  if (!val_json.is_number_unsigned()) {
    throw JsonResponseError::TypeMismatch(endpoint, key + " unsigned number",
                                          val_json);
  }
  return val_json;
}

template <>
double ParseAt(const std::string& endpoint, const nlohmann::json& json,
               const std::string& key) {
  const auto& val_json = CheckedAt(endpoint, json, key);
  if (val_json.is_null()) {
    return 0;
  }
  if (!val_json.is_number()) {
    throw JsonResponseError::TypeMismatch(endpoint, key + " number", val_json);
  }
  return val_json;
}

template <>
std::vector<std::string> ParseAt(const std::string& endpoint,
                                 const nlohmann::json& json,
                                 const std::string& key) {
  const auto& symbols_json = CheckedAt(endpoint, json, key);
  // if there's only one symbol, it returns a string not an array
  if (symbols_json.is_string()) {
    return {symbols_json};
  }
  if (!symbols_json.is_array()) {
    throw JsonResponseError::TypeMismatch(endpoint, key + " array", json);
  }
  return {symbols_json.begin(), symbols_json.end()};
}

template <>
date::year_month_day ParseAt(const std::string& endpoint,
                             const nlohmann::json& json,
                             const std::string& key) {
  std::string raw_start = detail::CheckedAt(endpoint, json, key);
  std::istringstream start_stream{raw_start};
  date::year_month_day start;
  start_stream >> date::parse("%F", start);
  if (start_stream.fail()) {
    throw JsonResponseError::TypeMismatch(endpoint, "YYYY-MM-DD date string",
                                          raw_start);
  }
  return start;
}
}  // namespace detail
}  // namespace databento
