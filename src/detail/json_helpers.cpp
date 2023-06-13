#include "databento/detail/json_helpers.hpp"

#include <numeric>  // accumulate

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
std::size_t ParseAt(const std::string& endpoint, const nlohmann::json& json,
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
  std::vector<std::string> res;
  res.reserve(symbols_json.size());
  for (const auto& item : symbols_json.items()) {
    res.emplace_back(item.value());
  }
  return res;
}

}  // namespace detail
}  // namespace databento
