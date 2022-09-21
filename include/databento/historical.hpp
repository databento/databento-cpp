#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <vector>

#include "batch.hpp"
#include "databento/symbology.hpp"
#include "enums.hpp"

namespace databento {
// TODO: Implement RAII StreamingToken and callback interface
class StreamingToken {};
using StreamingCallback = std::function<void()>;

class Historical {
  const std::string key_;
  const std::string gateway_;

 public:
  Historical(std::string key, HistoricalGateway gateway);

  // Getters

  const std::string& key() const { return key_; };
  const std::string& gateway() const { return gateway_; }

  // Batch API

  BatchJob BatchSubmitJob();
  std::vector<BatchJob> BatchListJobs();

  // Metadata API

  // Retrievs a mapping of publisher name to publisher ID.
  std::map<std::string, std::int32_t> MetadataListPublishers();
  std::vector<std::string> MetadataListDatasets();
  std::vector<std::string> MetadataListSchemas();
  // list_fields, list_encodings, and list_compressions don't seem useful in C++

  std::map<Schema, double> MetadataListUnitPrices();
  double MetadataListUnitPrice();
  std::size_t MetadataGetBillableSize();
  double MetadataGetCost();

  // Symbology API
  SymbologyResolution SymbologyResolve(const std::string& dataset,
                                       const std::string& symbol,
                                       SType stype_in, SType stype_out,
                                       const std::string& start_date,
                                       const std::string& end_date);
  SymbologyResolution SymbologyResolve(const std::string& dataset,
                                       const std::vector<std::string>& symbol,
                                       SType stype_in, SType stype_out,
                                       const std::string& start_date,
                                       const std::string& end_date);

  // Timeseries API
  // TODO: create request builder for default options
  StreamingToken TimeseriesStream(const std::string& dataset,
                                  const std::vector<std::string>& symbols,
                                  Schema schema,
                                  std::chrono::system_clock start,
                                  std::chrono::system_clock end, SType stype_in,
                                  SType stype_out, std::size_t limit,
                                  StreamingCallback callback);
};

class HistoricalBuilder {
  std::string key_;
  HistoricalGateway gateway_{HistoricalGateway::Nearest};

 public:
  HistoricalBuilder() = default;

  // Sets `key_` based on the environment variable DATABENTO_API_KEY.
  //
  // NOTE: This is not thread-safe if `std::setenv` is used elsewhere in the
  // program.
  HistoricalBuilder& keyFromEnv();
  HistoricalBuilder& key(std::string key);
  HistoricalBuilder& gateway(HistoricalGateway gateway);
  Historical Build();
};
}  // namespace databento
