#include "databento/historical.hpp"

#include <httplib.h>

#include <cstdlib>  // get_env
#include <nlohmann/json.hpp>
#include <numeric>  // accumulate
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>  // move

#include "databento/constants.hpp"
#include "databento/enums.hpp"

using databento::Historical;
using databento::HistoricalBuilder;

namespace {
std::string BuildMetadataPath(const char* slug) {
  return std::string{"/v"} + databento::kApiVersionStr + "/metadata" + slug;
}

void SetIfNotEmpty(httplib::Params* params, const std::string& key,
                   const std::string& value) {
  if (!value.empty()) {
    params->emplace(key, value);
  }
}

std::string QueryStringify(const std::vector<std::string>& strings) {
  return std::accumulate(strings.begin(), strings.end(), std::string{},
                         [](std::string acc, const std::string& symbol) {
                           return acc.empty() ? symbol
                                              : std::move(acc) + "," + symbol;
                         });
}
}  // namespace

Historical::Historical(std::string key, HistoricalGateway gateway)
    : key_{std::move(key)},
      gateway_{UrlFromGateway(gateway)},
      client_{key_, gateway_} {}

Historical::Historical(std::string key, std::string gateway, std::uint16_t port)
    : key_{std::move(key)},
      gateway_{std::move(gateway)},
      client_{key_, gateway_, port} {}

std::map<std::string, std::int32_t> Historical::MetadataListPublishers() {
  static const std::string kPath = ::BuildMetadataPath(".list_publishers");
  const nlohmann::json json = client_.GetJson(kPath, httplib::Params{});
  std::map<std::string, std::int32_t> publisher_to_pub_id;
  if (!json.is_object()) {
    throw std::runtime_error{
        std::string{"Expected JSON object response for ListPublishers, got "} +
        json.type_name()};
  }
  for (const auto& pair : json.items()) {
    if (!pair.value().is_number_integer()) {
      std::ostringstream err_msg;
      err_msg << "Expected number values in JSON object response for "
                 "ListPublishers, got "
              << pair.value().type_name() << " " << pair.value() << " for key "
              << pair.key();
      throw std::runtime_error{err_msg.str()};
    }
    publisher_to_pub_id.emplace(pair.key(), pair.value());
  }
  return publisher_to_pub_id;
}

std::vector<std::string> Historical::MetadataListDatasets() {
  return this->MetadataListDatasets({}, {});
}
std::vector<std::string> Historical::MetadataListDatasets(
    const std::string& start_date, const std::string& end_date) {
  static const std::string kPath = ::BuildMetadataPath(".list_datasets");
  httplib::Params params{};
  ::SetIfNotEmpty(&params, "start_date", start_date);
  ::SetIfNotEmpty(&params, "end_date", end_date);
  const nlohmann::json json = client_.GetJson(kPath, params);
  if (!json.is_array()) {
    throw std::runtime_error{
        std::string{"Expected JSON array response for ListDatasets, got "} +
        json.type_name()};
  }
  std::vector<std::string> datasets;
  datasets.reserve(json.size());
  for (const auto& item : json.items()) {
    if (!item.value().is_string()) {
      std::ostringstream err_msg;
      err_msg << "Expected string values in JSON array response for "
                 "ListDatasets, got "
              << item.value().type_name() << " " << item.value() << " at index "
              << item.key();
    }
    datasets.emplace_back(item.value());
  }
  return datasets;
}

std::vector<databento::Schema> Historical::MetadataListSchemas(
    const std::string& dataset) {
  return this->MetadataListSchemas(dataset, {}, {});
}
std::vector<databento::Schema> Historical::MetadataListSchemas(
    const std::string& dataset, const std::string& start_date,
    const std::string& end_date) {
  static const std::string kPath = ::BuildMetadataPath(".list_schemas");
  httplib::Params params{{"dataset", dataset}};
  ::SetIfNotEmpty(&params, "start_date", start_date);
  ::SetIfNotEmpty(&params, "end_date", end_date);
  const nlohmann::json json = client_.GetJson(kPath, params);
  if (!json.is_array()) {
    throw std::runtime_error{
        std::string{"Expected JSON array response for ListSchemas, got "} +
        json.type_name()};
  }
  std::vector<Schema> schemas;
  schemas.reserve(json.size());
  for (const auto& item : json.items()) {
    if (!item.value().is_string()) {
      std::ostringstream err_msg;
      err_msg << "Expected string values in JSON array response for "
                 "ListSchemas, got "
              << item.value().type_name() << " " << item.value() << " at index "
              << item.key();
    }
    schemas.emplace_back(FromString<Schema>(item.value()));
  }
  return schemas;
}

static const std::string kListUnitPricesPath =
    ::BuildMetadataPath(".list_unit_prices");

std::map<databento::FeedMode, std::map<databento::Schema, double>>
Historical::MetadataListUnitPrices(const std::string& dataset) {
  const nlohmann::json json = client_.GetJson(
      kListUnitPricesPath, httplib::Params{{"dataset", dataset}});
  if (!json.is_object()) {
    throw std::runtime_error{
        std::string{"Expected JSON object response for ListUnitPrices, got "} +
        json.type_name()};
  }
  std::map<FeedMode, std::map<Schema, double>> prices;
  for (const auto& mode_and_prices : json.items()) {
    if (!mode_and_prices.value().is_object()) {
      std::ostringstream err_msg;
      err_msg << "Expected object values in JSON object response for "
                 "ListUnitPrices, got "
              << mode_and_prices.value().type_name() << " "
              << mode_and_prices.value() << " at index "
              << mode_and_prices.key();
    }
    decltype(prices)::mapped_type schema_prices;
    for (const auto& schema_and_price : mode_and_prices.value().items()) {
      if (!schema_and_price.value().is_number()) {
        std::ostringstream err_msg;
        err_msg << "Expected nested number values in JSON object response for "
                   "ListUnitPrices, got "
                << schema_and_price.value().type_name() << " "
                << schema_and_price.value() << " at index "
                << schema_and_price.key();
      }
      schema_prices.emplace(FromString<Schema>(schema_and_price.key()),
                            schema_and_price.value());
    }
    prices.emplace(FromString<FeedMode>(mode_and_prices.key()),
                   std::move(schema_prices));
  }
  return prices;
}

std::map<databento::Schema, double> Historical::MetadataListUnitPrices(
    const std::string& dataset, databento::FeedMode mode) {
  const std::string mode_str = ToString(mode);
  const nlohmann::json json = client_.GetJson(
      kListUnitPricesPath,
      httplib::Params{{"dataset", dataset}, {"mode", mode_str}});
  if (!json.is_object()) {
    throw std::runtime_error{
        std::string{"Expected JSON object response for ListUnitPrices, got "} +
        json.type_name()};
  }
  const auto& json_map = json.at(mode_str);
  std::map<Schema, double> prices;
  for (const auto& item : json_map.items()) {
    if (!item.value().is_number()) {
      std::ostringstream err_msg;
      err_msg << "Expected number values in JSON object response for "
                 "ListUnitPrices, got "
              << item.value().type_name() << " " << item.value() << " at index "
              << item.key();
    }
    prices.emplace(FromString<Schema>(item.key()), item.value());
  }
  return prices;
}

std::map<databento::FeedMode, double> Historical::MetadataListUnitPrices(
    const std::string& dataset, databento::Schema schema) {
  const std::string schema_str = ToString(schema);
  const nlohmann::json json = client_.GetJson(
      kListUnitPricesPath,
      httplib::Params{{"dataset", dataset}, {"schema", schema_str}});
  if (!json.is_object()) {
    throw std::runtime_error{
        std::string{"Expected JSON object response for ListUnitPrices, got "} +
        json.type_name()};
  }
  std::map<FeedMode, double> prices;
  for (const auto& mode_and_prices : json.items()) {
    if (!mode_and_prices.value().is_object()) {
      std::ostringstream err_msg;
      err_msg << "Expected object values in JSON object response for "
                 "ListUnitPrices, got "
              << mode_and_prices.value().type_name() << " "
              << mode_and_prices.value() << " at index "
              << mode_and_prices.key();
    }
    auto const price_json = mode_and_prices.value().at(schema_str);
    if (!price_json.is_number()) {
      std::ostringstream err_msg;
      err_msg << "Expected number values in nested JSON object response for "
                 "ListUnitPrices, got "
              << price_json.type_name() << " " << price_json << " at index "
              << mode_and_prices.key();
    }

    prices.emplace(FromString<FeedMode>(mode_and_prices.key()), price_json);
  }
  return prices;
}

double Historical::MetadataListUnitPrices(const std::string& dataset,
                                          FeedMode mode, Schema schema) {
  const nlohmann::json json = client_.GetJson(
      kListUnitPricesPath, httplib::Params{{"dataset", dataset},
                                           {"mode", ToString(mode)},
                                           {"schema", ToString(schema)}});
  if (!json.is_number()) {
    throw std::runtime_error{
        std::string{"Expected JSON number response for ListUnitPrices, got "} +
        json.type_name()};
  }
  return json;
}

std::size_t Historical::MetadataGetBillableSize(const std::string& dataset,
                                                const std::string& start,
                                                const std::string& end) {
  return this->MetadataGetBillableSize(dataset, start, end, {}, Schema::Trades,
                                       SType::Native, {});
}

std::size_t Historical::MetadataGetBillableSize(
    const std::string& dataset, const std::string& start,
    const std::string& end, const std::vector<std::string>& symbols,
    Schema schema, SType stype_in, std::size_t limit) {
  static const std::string kPath = ::BuildMetadataPath(".get_billable_size");
  httplib::Params params{{"dataset", dataset},
                         {"schema", ToString(schema)},
                         {"stype_in", ToString(stype_in)}};
  ::SetIfNotEmpty(&params, "start", start);
  ::SetIfNotEmpty(&params, "end", end);
  if (limit > 0) {
    params.emplace("limit", std::to_string(limit));
  }
  if (!symbols.empty()) {
    params.emplace("symbols", ::QueryStringify(symbols));
  }
  const nlohmann::json json = client_.GetJson(kPath, params);
  if (!json.is_number()) {
    throw std::runtime_error{
        std::string{"Expected JSON number response for GetBillableSize, got "} +
        json.type_name()};
  }
  return json;
}

double Historical::MetadataGetCost(const std::string& dataset,
                                   const std::string& start,
                                   const std::string& end) {
  return this->MetadataGetCost(dataset, start, end,
                               FeedMode::HistoricalStreaming, {},
                               Schema::Trades, SType::Native, {});
}

double Historical::MetadataGetCost(const std::string& dataset,
                                   const std::string& start,
                                   const std::string& end, FeedMode mode,
                                   const std::vector<std::string>& symbols,
                                   Schema schema, SType stype_in,
                                   std::size_t limit) {
  static const std::string kPath = ::BuildMetadataPath(".get_cost");
  httplib::Params params{{"dataset", dataset},
                         {"mode", ToString(mode)},
                         {"schema", ToString(schema)},
                         {"stype_in", ToString(stype_in)}};
  ::SetIfNotEmpty(&params, "start", start);
  ::SetIfNotEmpty(&params, "end", end);
  if (!symbols.empty()) {
    params.emplace("symbols", ::QueryStringify(symbols));
  }
  if (limit > 0) {
    params.emplace("limit", std::to_string(limit));
  }
  const nlohmann::json json = client_.GetJson(kPath, params);
  if (!json.is_number()) {
    throw std::runtime_error{
        std::string{"Expected JSON number response for GetCost, got "} +
        json.type_name()};
  }
  return json;
}

HistoricalBuilder& HistoricalBuilder::keyFromEnv() {
  char const* env_key = std::getenv("DATABENTO_API_KEY");
  if (env_key == nullptr) {
    throw std::runtime_error{
        "Expected environment variable DATABENTO_API_KEY to be set"};
  }
  return this->key(env_key);
}

HistoricalBuilder& HistoricalBuilder::key(std::string key) {
  key_ = std::move(key);
  return *this;
}

HistoricalBuilder& HistoricalBuilder::gateway(HistoricalGateway gateway) {
  gateway_ = gateway;
  return *this;
}

Historical HistoricalBuilder::Build() {
  if (key_.empty()) {
    throw std::logic_error{"key is unset"};
  }
  return Historical{std::move(key_), gateway_};
}
