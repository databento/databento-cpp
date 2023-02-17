#include "databento/historical.hpp"

#include <dirent.h>  // opendir
#include <httplib.h>
#include <nlohmann/json.hpp>

#include <algorithm>  // find_if
#include <atomic>     // atomic<bool>
#include <cstdlib>    // get_env
#include <exception>  // exception, exception_ptr
#include <fstream>    // ofstream
#include <ios>        // ios::binary
#include <numeric>    // accumulate
#include <string>
#include <utility>  // move

#include "databento/constants.hpp"
#include "databento/datetime.hpp"
#include "databento/dbn_decoder.hpp"
#include "databento/detail/scoped_thread.hpp"
#include "databento/detail/shared_channel.hpp"
#include "databento/enums.hpp"
#include "databento/exceptions.hpp"  // Exception, JsonResponseError
#include "databento/file_bento.hpp"
#include "databento/metadata.hpp"
#include "databento/timeseries.hpp"

using databento::Historical;

namespace {
std::string BuildBatchPath(const char* slug) {
  return std::string{"/v"} + databento::kApiVersionStr + "/batch" + slug;
}

std::string BuildMetadataPath(const char* slug) {
  return std::string{"/v"} + databento::kApiVersionStr + "/metadata" + slug;
}

std::string BuildSymbologyPath(const char* slug) {
  return std::string{"/v"} + databento::kApiVersionStr + "/symbology" + slug;
}

std::string BuildTimeseriesPath(const char* slug) {
  return std::string{"/v"} + databento::kApiVersionStr + "/timeseries" + slug;
}

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

template <typename T>
void SetIfPositive(httplib::Params* params, const std::string& key,
                   const T value) {
  if (value > 0) {
    params->emplace(key, std::to_string(value));
  }
}

using databento::JsonResponseError;

const nlohmann::json& CheckedAt(const std::string& endpoint,
                                const nlohmann::json& json,
                                const std::string& key) {
  if (json.contains(key)) {
    return json.at(key);
  }
  throw JsonResponseError::MissingKey(endpoint, key);
}

template <typename T>
T FromCheckedAtString(const std::string& endpoint, const nlohmann::json& json,
                      const std::string& key) {
  const auto& val_json = ::CheckedAt(endpoint, json, key);
  if (!val_json.is_string()) {
    throw JsonResponseError::TypeMismatch(endpoint, key + " string", val_json);
  }
  return databento::FromString<T>(val_json);
}

template <typename T>
T ParseAt(const std::string& endpoint, const nlohmann::json& json,
          const std::string& key);

template <>
bool ParseAt(const std::string& endpoint, const nlohmann::json& json,
             const std::string& key) {
  const auto& val_json = ::CheckedAt(endpoint, json, key);
  if (!val_json.is_boolean()) {
    throw JsonResponseError::TypeMismatch(endpoint, key + " bool", val_json);
  }
  return val_json;
}

template <>
std::string ParseAt(const std::string& endpoint, const nlohmann::json& json,
                    const std::string& key) {
  const auto& val_json = ::CheckedAt(endpoint, json, key);
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
  const auto& val_json = ::CheckedAt(endpoint, json, key);
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
  const auto& val_json = ::CheckedAt(endpoint, json, key);
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
  const auto& symbols_json = ::CheckedAt(endpoint, json, key);
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

constexpr auto kDefaultSTypeIn = databento::SType::Native;
constexpr auto kDefaultSTypeOut = databento::SType::ProductId;

databento::BatchJob Parse(const std::string& endpoint,
                          const nlohmann::json& json) {
  using databento::Compression;
  using databento::Delivery;
  using databento::Encoding;
  using databento::JobState;
  using databento::Packaging;
  using databento::Schema;
  using databento::SplitDuration;
  using databento::SType;

  if (!json.is_object()) {
    throw JsonResponseError::TypeMismatch(endpoint, "object", json);
  }
  databento::BatchJob res;
  res.id = ::CheckedAt(endpoint, json, "id");
  res.user_id = ParseAt<std::string>(endpoint, json, "user_id");
  res.bill_id = ParseAt<std::string>(endpoint, json, "bill_id");
  res.cost = ParseAt<double>(endpoint, json, "cost");
  res.dataset = ParseAt<std::string>(endpoint, json, "dataset");
  res.symbols = ParseAt<std::vector<std::string>>(endpoint, json, "symbols");
  res.stype_in = FromCheckedAtString<SType>(endpoint, json, "stype_in");
  res.stype_out = FromCheckedAtString<SType>(endpoint, json, "stype_out");
  res.schema = FromCheckedAtString<Schema>(endpoint, json, "schema");
  res.start = ParseAt<std::string>(endpoint, json, "start");
  res.end = ParseAt<std::string>(endpoint, json, "end");
  res.limit = ParseAt<std::size_t>(endpoint, json, "limit");
  res.encoding = FromCheckedAtString<Encoding>(endpoint, json, "encoding");
  res.compression =
      FromCheckedAtString<Compression>(endpoint, json, "compression");
  res.split_duration =
      FromCheckedAtString<SplitDuration>(endpoint, json, "split_duration");
  res.split_size = ParseAt<std::size_t>(endpoint, json, "split_size");
  res.split_symbols = ParseAt<bool>(endpoint, json, "split_symbols");
  res.packaging = FromCheckedAtString<Packaging>(endpoint, json, "packaging");
  res.delivery = FromCheckedAtString<Delivery>(endpoint, json, "delivery");
  res.record_count = ParseAt<std::size_t>(endpoint, json, "record_count");
  res.billed_size = ParseAt<std::size_t>(endpoint, json, "billed_size");
  res.actual_size = ParseAt<std::size_t>(endpoint, json, "actual_size");
  res.package_size = ParseAt<std::size_t>(endpoint, json, "package_size");
  res.state = FromCheckedAtString<JobState>(endpoint, json, "state");
  res.ts_received = ParseAt<std::string>(endpoint, json, "ts_received");
  res.ts_queued = ParseAt<std::string>(endpoint, json, "ts_queued");
  res.ts_process_start =
      ParseAt<std::string>(endpoint, json, "ts_process_start");
  res.ts_process_done = ParseAt<std::string>(endpoint, json, "ts_process_done");
  res.ts_expiration = ParseAt<std::string>(endpoint, json, "ts_expiration");
  return res;
}

std::string PathJoin(const std::string& dir, const std::string& path) {
  if (dir[dir.length() - 1] == '/') {
    return dir + path;
  }
  return dir + '/' + path;
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

static const std::string kBatchSubmitJobEndpoint = "Historical::BatchSubmitJob";

databento::BatchJob Historical::BatchSubmitJob(
    const std::string& dataset, UnixNanos start, UnixNanos end,
    const std::vector<std::string>& symbols, Schema schema) {
  return this->BatchSubmitJob(dataset, start, end, symbols, schema,
                              SplitDuration::Day, {}, Packaging::None,
                              Delivery::Download, kDefaultSTypeIn,
                              kDefaultSTypeOut, {});
}
databento::BatchJob Historical::BatchSubmitJob(
    const std::string& dataset, const std::string& start,
    const std::string& end, const std::vector<std::string>& symbols,
    Schema schema) {
  return this->BatchSubmitJob(dataset, start, end, symbols, schema,
                              SplitDuration::Day, {}, Packaging::None,
                              Delivery::Download, kDefaultSTypeIn,
                              kDefaultSTypeOut, {});
}
databento::BatchJob Historical::BatchSubmitJob(
    const std::string& dataset, UnixNanos start, UnixNanos end,
    const std::vector<std::string>& symbols, Schema schema,
    SplitDuration split_duration, std::size_t split_size, Packaging packaging,
    Delivery delivery, SType stype_in, SType stype_out, std::size_t limit) {
  httplib::Params params{
      {"dataset", dataset},
      {"start", ToString(start)},
      {"end", ToString(end)},
      {"symbols", JoinSymbolStrings(kBatchSubmitJobEndpoint, symbols)},
      {"schema", ToString(schema)},
      {"encoding", "dbn"},
      {"split_duration", ToString(split_duration)},
      {"packaging", ToString(packaging)},
      {"delivery", ToString(delivery)},
      {"stype_in", ToString(stype_in)},
      {"stype_out", ToString(stype_out)}};
  ::SetIfPositive(&params, "split_size", split_size);
  ::SetIfPositive(&params, "limit", limit);
  return this->BatchSubmitJob(params);
}
databento::BatchJob Historical::BatchSubmitJob(
    const std::string& dataset, const std::string& start,
    const std::string& end, const std::vector<std::string>& symbols,
    Schema schema, SplitDuration split_duration, std::size_t split_size,
    Packaging packaging, Delivery delivery, SType stype_in, SType stype_out,
    std::size_t limit) {
  httplib::Params params{
      {"dataset", dataset},
      {"start", start},
      {"end", end},
      {"symbols", JoinSymbolStrings(kBatchSubmitJobEndpoint, symbols)},
      {"schema", ToString(schema)},
      {"encoding", "dbn"},
      {"split_duration", ToString(split_duration)},
      {"packaging", ToString(packaging)},
      {"delivery", ToString(delivery)},
      {"stype_in", ToString(stype_in)},
      {"stype_out", ToString(stype_out)}};
  ::SetIfPositive(&params, "split_size", split_size);
  ::SetIfPositive(&params, "limit", limit);
  return this->BatchSubmitJob(params);
}
databento::BatchJob Historical::BatchSubmitJob(const httplib::Params& params) {
  static const std::string kPath = ::BuildBatchPath(".submit_job");
  const nlohmann::json json = client_.PostJson(kPath, params);
  return ::Parse("BatchSubmitJob", json);
}

std::vector<databento::BatchJob> Historical::BatchListJobs() {
  static const std::vector<JobState> kDefaultStates = {
      JobState::Received, JobState::Queued, JobState::Processing,
      JobState::Done};
  return this->BatchListJobs(kDefaultStates, UnixNanos{});
}
std::vector<databento::BatchJob> Historical::BatchListJobs(
    const std::vector<databento::JobState>& states, UnixNanos since) {
  httplib::Params params;
  ::SetIfNotEmpty(&params, "states", states);
  ::SetIfPositive(&params, "since", since.time_since_epoch().count());
  return this->BatchListJobs(params);
}
std::vector<databento::BatchJob> Historical::BatchListJobs(
    const std::vector<databento::JobState>& states, const std::string& since) {
  httplib::Params params;
  ::SetIfNotEmpty(&params, "states", states);
  ::SetIfNotEmpty(&params, "since", since);
  return this->BatchListJobs(params);
}
std::vector<databento::BatchJob> Historical::BatchListJobs(
    const httplib::Params& params) {
  static const std::string kEndpoint = "Historical::BatchListJobs";
  static const std::string kPath = ::BuildBatchPath(".list_jobs");
  const nlohmann::json json = client_.GetJson(kPath, params);
  if (!json.is_array()) {
    throw JsonResponseError::TypeMismatch(kEndpoint, "array", json);
  }
  std::vector<BatchJob> jobs;
  jobs.reserve(json.size());
  for (const auto& job_json : json.items()) {
    jobs.emplace_back(::Parse(kEndpoint, job_json.value()));
  }
  return jobs;
}

std::vector<databento::BatchFileDesc> Historical::BatchListFiles(
    const std::string& job_id) {
  static const std::string kEndpoint = "Historical::BatchListFiles";
  static const std::string kPath = ::BuildBatchPath(".list_files");

  const nlohmann::json json =
      client_.GetJson(kPath, httplib::Params{{"job_id", job_id}});
  if (!json.is_array()) {
    throw JsonResponseError::TypeMismatch(kEndpoint, "array", json);
  }
  std::vector<BatchFileDesc> files;
  files.reserve(json.size());
  for (const auto& file_json : json.items()) {
    const auto& file_obj = file_json.value();
    BatchFileDesc file_desc;
    file_desc.filename =
        ::ParseAt<std::string>(kEndpoint, file_obj, "filename");
    file_desc.size = ::ParseAt<std::size_t>(kEndpoint, file_obj, "size");
    file_desc.hash = ::ParseAt<std::string>(kEndpoint, file_obj, "hash");
    const auto& url_obj = ::CheckedAt(kEndpoint, file_obj, "urls");
    file_desc.https_url = ::ParseAt<std::string>(kEndpoint, url_obj, "https");
    file_desc.ftp_url = ::ParseAt<std::string>(kEndpoint, url_obj, "ftp");
    files.emplace_back(file_desc);
  }
  return files;
}

void Historical::BatchDownload(const std::string& output_dir,
                               const std::string& job_id) {
  if (::opendir(output_dir.c_str()) == nullptr) {
    const int ret = ::mkdir(output_dir.c_str(), 0777);
    if (ret != 0) {
      throw Exception{std::string{"Unable to create output directory: "} +
                      ::strerror(errno)};
    }
  }

  const auto file_descs = BatchListFiles(job_id);
  for (const auto& file_desc : file_descs) {
    const std::string output_path = PathJoin(output_dir, file_desc.filename);
    DownloadFile(file_desc.https_url, output_path);
  }
}
void Historical::BatchDownload(const std::string& output_dir,
                               const std::string& job_id,
                               const std::string& filename_to_download) {
  const auto file_descs = BatchListFiles(job_id);
  const auto file_desc_it =
      std::find_if(file_descs.begin(), file_descs.end(),
                   [&filename_to_download](const BatchFileDesc& file_desc) {
                     return file_desc.filename == filename_to_download;
                   });
  if (file_desc_it == file_descs.end()) {
    throw InvalidArgumentError{"Historical::BatchDownload",
                               "filename_to_download",
                               "Filename not found for batch job " + job_id};
  }
  const std::string output_path = PathJoin(output_dir, file_desc_it->filename);
  DownloadFile(file_desc_it->https_url, output_path);
}

void Historical::DownloadFile(const std::string& url,
                              const std::string& output_path) {
  static const std::string kEndpoint = "Historical::BatchDownload";
  // extract path from URL
  const auto protocol_divider = url.find("://");
  std::string path;
  if (protocol_divider == std::string::npos) {
    const auto slash = url.find_first_of('/');
    if (slash == std::string::npos) {
      throw InvalidArgumentError{"Historical::DownloadFile", "url",
                                 "No slashes"};
    }
    path = url.substr(slash);
  } else {
    const auto slash = url.find('/', protocol_divider + 3);
    if (slash == std::string::npos) {
      throw InvalidArgumentError{"Historical::DownloadFile", "url",
                                 "No slashes"};
    }
    path = url.substr(slash);
  }

  client_.GetRawStream(
      path, {}, [&output_path](const char* data, std::size_t length) {
        std::ofstream out_file{output_path};
        out_file.write(data, static_cast<std::streamsize>(length));
        return KeepGoing::Continue;
      });
}

std::map<std::string, std::int32_t> Historical::MetadataListPublishers() {
  static const std::string kEndpoint = "Historical::MetadataListPublishers";
  static const std::string kPath = ::BuildMetadataPath(".list_publishers");
  const nlohmann::json json = client_.GetJson(kPath, httplib::Params{});
  std::map<std::string, std::int32_t> publisher_to_pub_id;
  if (!json.is_object()) {
    throw JsonResponseError::TypeMismatch(kEndpoint, "object", json);
  }
  for (const auto& pair : json.items()) {
    if (!pair.value().is_number_integer()) {
      throw JsonResponseError::TypeMismatch(kEndpoint, "integer number",
                                            pair.key(), pair.value());
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
  static const std::string kEndpoint = "Historical::MetadataListDatasets";
  static const std::string kPath = ::BuildMetadataPath(".list_datasets");
  httplib::Params params{};
  ::SetIfNotEmpty(&params, "start_date", start_date);
  ::SetIfNotEmpty(&params, "end_date", end_date);
  const nlohmann::json json = client_.GetJson(kPath, params);
  if (!json.is_array()) {
    throw JsonResponseError::TypeMismatch(kEndpoint, "array", json);
  }
  std::vector<std::string> datasets;
  datasets.reserve(json.size());
  for (const auto& item : json.items()) {
    if (!item.value().is_string()) {
      // `key()` in an array is the index
      throw JsonResponseError::TypeMismatch(kEndpoint, "string", item.key(),
                                            item.value());
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
  static const std::string kEndpoint = "Historical::MetadataListSchemas";
  static const std::string kPath = ::BuildMetadataPath(".list_schemas");
  httplib::Params params{{"dataset", dataset}};
  ::SetIfNotEmpty(&params, "start_date", start_date);
  ::SetIfNotEmpty(&params, "end_date", end_date);
  const nlohmann::json json = client_.GetJson(kPath, params);
  if (!json.is_array()) {
    throw JsonResponseError::TypeMismatch(kEndpoint, "array", json);
  }
  std::vector<Schema> schemas;
  schemas.reserve(json.size());
  for (const auto& item : json.items()) {
    if (!item.value().is_string()) {
      throw JsonResponseError::TypeMismatch(kEndpoint, "string", item.key(),
                                            item.value());
    }
    schemas.emplace_back(FromString<Schema>(item.value()));
  }
  return schemas;
}

databento::FieldsByDatasetEncodingAndSchema Historical::MetadataListFields() {
  return this->MetadataListFields(httplib::Params{});
}
databento::FieldsByDatasetEncodingAndSchema Historical::MetadataListFields(
    const std::string& dataset) {
  httplib::Params params;
  ::SetIfNotEmpty(&params, "dataset", dataset);
  return this->MetadataListFields(params);
}
databento::FieldsByDatasetEncodingAndSchema Historical::MetadataListFields(
    const std::string& dataset, Encoding encoding, Schema schema) {
  httplib::Params params{{"encoding", ToString(encoding)},
                         {"schema", ToString(schema)}};
  ::SetIfNotEmpty(&params, "dataset", dataset);
  return this->MetadataListFields(params);
}
databento::FieldsByDatasetEncodingAndSchema Historical::MetadataListFields(
    const httplib::Params& params) {
  static const std::string kEndpoint = "Historical::MetadataListFields";
  static const std::string kPath = ::BuildMetadataPath(".list_fields");
  const nlohmann::json json = client_.GetJson(kPath, params);
  if (!json.is_object()) {
    throw JsonResponseError::TypeMismatch(kEndpoint, "object", json);
  }
  FieldsByDatasetEncodingAndSchema fields;
  for (const auto& dataset_and_fields : json.items()) {
    if (!dataset_and_fields.value().is_object()) {
      throw JsonResponseError::TypeMismatch(kEndpoint, "object",
                                            dataset_and_fields.key(),
                                            dataset_and_fields.value());
    }
    FieldsByEncodingAndSchema fields_by_encoding_and_schema;
    for (const auto& encoding_and_fields : dataset_and_fields.value().items()) {
      if (!encoding_and_fields.value().is_object()) {
        throw JsonResponseError::TypeMismatch(kEndpoint, "nested object",
                                              encoding_and_fields.key(),
                                              encoding_and_fields.value());
      }
      FieldsByEncodingAndSchema::mapped_type fields_by_schema;
      for (const auto& schema_and_fields :
           encoding_and_fields.value().items()) {
        if (!schema_and_fields.value().is_object()) {
          throw JsonResponseError::TypeMismatch(
              kEndpoint, "nested nested object", schema_and_fields.key(),
              schema_and_fields.value());
        }
        FieldDefinition field_def;
        for (const auto& field_and_type : schema_and_fields.value().items()) {
          if (!field_and_type.value().is_string()) {
            throw JsonResponseError::TypeMismatch(kEndpoint, "string",
                                                  field_and_type.key(),
                                                  field_and_type.value());
          }
          field_def.emplace(field_and_type.key(), field_and_type.value());
        }
        fields_by_schema.emplace(FromString<Schema>(schema_and_fields.key()),
                                 std::move(field_def));
      }
      fields_by_encoding_and_schema.emplace(
          FromString<Encoding>(encoding_and_fields.key()),
          std::move(fields_by_schema));
    }
    fields.emplace(dataset_and_fields.key(),
                   std::move(fields_by_encoding_and_schema));
  }
  return fields;
}

std::vector<databento::Encoding> Historical::MetadataListEncodings() {
  static const std::string kEndpoint = "Historical::MetadataListEncodings";
  static const std::string kPath = ::BuildMetadataPath(".list_encodings");
  const nlohmann::json json = client_.GetJson(kPath, {});
  if (!json.is_array()) {
    throw JsonResponseError::TypeMismatch(kEndpoint, "array", json);
  }
  std::vector<Encoding> encodings;
  for (const auto& encoding_json : json.items()) {
    if (!encoding_json.value().is_string()) {
      throw JsonResponseError::TypeMismatch(
          kEndpoint, "string", encoding_json.key(), encoding_json.value());
    }
    encodings.emplace_back(FromString<Encoding>(encoding_json.value()));
  }
  return encodings;
}

std::vector<databento::Compression> Historical::MetadataListCompressions() {
  static const std::string kEndpoint = "Historical::MetadataListCompressions";
  static const std::string kPath = ::BuildMetadataPath(".list_compressions");
  const nlohmann::json json = client_.GetJson(kPath, {});
  if (!json.is_array()) {
    throw JsonResponseError::TypeMismatch(kEndpoint, "array", json);
  }
  std::vector<Compression> compressions;
  for (const auto& compression_json : json.items()) {
    if (!compression_json.value().is_string()) {
      throw JsonResponseError::TypeMismatch(kEndpoint, "string",
                                            compression_json.key(),
                                            compression_json.value());
    }
    compressions.emplace_back(
        FromString<Compression>(compression_json.value()));
  }
  return compressions;
}

static const std::string kListUnitPricesEndpoint =
    "Historical::MetadataListUnitPrices";
static const std::string kListUnitPricesPath =
    ::BuildMetadataPath(".list_unit_prices");

std::map<databento::FeedMode, std::map<databento::Schema, double>>
Historical::MetadataListUnitPrices(const std::string& dataset) {
  const nlohmann::json json = client_.GetJson(
      kListUnitPricesPath, httplib::Params{{"dataset", dataset}});
  if (!json.is_object()) {
    throw JsonResponseError::TypeMismatch(kListUnitPricesEndpoint, "object",
                                          json);
  }
  std::map<FeedMode, std::map<Schema, double>> prices;
  for (const auto& mode_and_prices : json.items()) {
    if (!mode_and_prices.value().is_object()) {
      throw JsonResponseError::TypeMismatch(kListUnitPricesEndpoint, "object",
                                            mode_and_prices.key(),
                                            mode_and_prices.value());
    }
    decltype(prices)::mapped_type schema_prices;
    for (const auto& schema_and_price : mode_and_prices.value().items()) {
      if (!schema_and_price.value().is_number()) {
        throw JsonResponseError::TypeMismatch(
            kListUnitPricesEndpoint, "nested number", schema_and_price.key(),
            schema_and_price.value());
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
    throw JsonResponseError::TypeMismatch(kListUnitPricesEndpoint, "object",
                                          json);
  }
  const auto& json_map = ::CheckedAt(kListUnitPricesEndpoint, json, mode_str);
  std::map<Schema, double> prices;
  for (const auto& item : json_map.items()) {
    if (!item.value().is_number()) {
      throw JsonResponseError::TypeMismatch(kListUnitPricesEndpoint, "number",
                                            item.key(), item.value());
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
    throw JsonResponseError::TypeMismatch(kListUnitPricesEndpoint, "object",
                                          json);
  }
  std::map<FeedMode, double> prices;
  for (const auto& mode_and_prices : json.items()) {
    if (!mode_and_prices.value().is_object()) {
      throw JsonResponseError::TypeMismatch(kListUnitPricesEndpoint, "object",
                                            mode_and_prices.key(),
                                            mode_and_prices.value());
    }
    const auto& price_json = ::CheckedAt("Historical::MetadataListUnitPrices",
                                         mode_and_prices.value(), schema_str);
    if (!price_json.is_number()) {
      throw JsonResponseError::TypeMismatch(kListUnitPricesEndpoint, "number",
                                            price_json);
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
    throw JsonResponseError::TypeMismatch("Historical::MetadataListUnitPrices",
                                          "number", json);
  }
  return json;
}

databento::DatasetConditionInfo Historical::MetadataGetDatasetCondition(
    const std::string& dataset, const std::string& start_date,
    const std::string& end_date) {
  static const std::string kEndpoint =
      "Historical::MetadataGetDatasetCondition";
  static const std::string kPath =
      ::BuildMetadataPath(".get_dataset_condition");

  const nlohmann::json json =
      client_.GetJson(kPath, httplib::Params{{"dataset", dataset},
                                             {"start_date", start_date},
                                             {"end_date", end_date}});
  if (!json.is_object()) {
    throw JsonResponseError::TypeMismatch(kEndpoint, "object", json);
  }
  const auto& details_json = CheckedAt(kEndpoint, json, "details");
  if (!details_json.is_array()) {
    throw JsonResponseError::TypeMismatch(kEndpoint, "details array", json);
  }
  std::vector<DatasetConditionDetail> details;
  details.reserve(details_json.size());
  for (const auto& detail_json : details_json.items()) {
    details.emplace_back(DatasetConditionDetail{
        ParseAt<std::string>(kEndpoint, detail_json.value(), "date"),
        FromCheckedAtString<DatasetCondition>(kEndpoint, detail_json.value(),
                                              "condition")});
  }
  return {FromCheckedAtString<DatasetCondition>(kEndpoint, json, "condition"),
          std::move(details),
          ParseAt<std::string>(kEndpoint, json, "adjusted_start_date"),
          ParseAt<std::string>(kEndpoint, json, "adjusted_end_date")};
}

static const std::string kMetadataGetRecordCountEndpoint =
    "Historical::MetadataGetRecordCount";

std::size_t Historical::MetadataGetRecordCount(
    const std::string& dataset, UnixNanos start, UnixNanos end,
    const std::vector<std::string>& symbols, Schema schema) {
  return this->MetadataGetRecordCount(dataset, start, end, symbols, schema,
                                      kDefaultSTypeIn, {});
}
std::size_t Historical::MetadataGetRecordCount(
    const std::string& dataset, const std::string& start,
    const std::string& end, const std::vector<std::string>& symbols,
    Schema schema) {
  return this->MetadataGetRecordCount(dataset, start, end, symbols, schema,
                                      kDefaultSTypeIn, {});
}
std::size_t Historical::MetadataGetRecordCount(
    const std::string& dataset, UnixNanos start, UnixNanos end,
    const std::vector<std::string>& symbols, Schema schema, SType stype_in,
    std::size_t limit) {
  httplib::Params params{
      {"dataset", dataset},
      {"start", ToString(start)},
      {"end", ToString(end)},
      {"symbols", JoinSymbolStrings(kMetadataGetRecordCountEndpoint, symbols)},
      {"schema", ToString(schema)},
      {"stype_in", ToString(stype_in)}};
  ::SetIfPositive(&params, "limit", limit);
  return this->MetadataGetRecordCount(params);
}
std::size_t Historical::MetadataGetRecordCount(
    const std::string& dataset, const std::string& start,
    const std::string& end, const std::vector<std::string>& symbols,
    Schema schema, SType stype_in, std::size_t limit) {
  httplib::Params params{
      {"dataset", dataset},
      {"start", start},
      {"end", end},
      {"symbols", JoinSymbolStrings(kMetadataGetRecordCountEndpoint, symbols)},
      {"schema", ToString(schema)},
      {"stype_in", ToString(stype_in)}};
  ::SetIfPositive(&params, "limit", limit);
  return this->MetadataGetRecordCount(params);
}
std::size_t Historical::MetadataGetRecordCount(const httplib::Params& params) {
  static const std::string kPath = ::BuildMetadataPath(".get_record_count");
  const nlohmann::json json = client_.GetJson(kPath, params);
  if (!json.is_number_unsigned()) {
    throw JsonResponseError::TypeMismatch("Historical::MetadataGetRecordCount",
                                          "unsigned number", json);
  }
  return json;
}

static const std::string kMetadataGetBillableSizeEndpoint =
    "Historical::MetadataGetBillableSize";

std::size_t Historical::MetadataGetBillableSize(
    const std::string& dataset, UnixNanos start, UnixNanos end,
    const std::vector<std::string>& symbols, Schema schema) {
  return this->MetadataGetBillableSize(dataset, start, end, symbols, schema,
                                       kDefaultSTypeIn, {});
}
std::size_t Historical::MetadataGetBillableSize(
    const std::string& dataset, const std::string& start,
    const std::string& end, const std::vector<std::string>& symbols,
    Schema schema) {
  return this->MetadataGetBillableSize(dataset, start, end, symbols, schema,
                                       kDefaultSTypeIn, {});
}
std::size_t Historical::MetadataGetBillableSize(
    const std::string& dataset, UnixNanos start, UnixNanos end,
    const std::vector<std::string>& symbols, Schema schema, SType stype_in,
    std::size_t limit) {
  httplib::Params params{
      {"dataset", dataset},
      {"start", ToString(start)},
      {"end", ToString(end)},
      {"symbols", JoinSymbolStrings(kMetadataGetBillableSizeEndpoint, symbols)},
      {"schema", ToString(schema)},
      {"stype_in", ToString(stype_in)}};
  ::SetIfPositive(&params, "limit", limit);
  return this->MetadataGetBillableSize(params);
}
std::size_t Historical::MetadataGetBillableSize(
    const std::string& dataset, const std::string& start,
    const std::string& end, const std::vector<std::string>& symbols,
    Schema schema, SType stype_in, std::size_t limit) {
  httplib::Params params{
      {"dataset", dataset},
      {"start", start},
      {"end", end},
      {"symbols", JoinSymbolStrings(kMetadataGetBillableSizeEndpoint, symbols)},
      {"schema", ToString(schema)},
      {"stype_in", ToString(stype_in)}};
  ::SetIfPositive(&params, "limit", limit);
  return this->MetadataGetBillableSize(params);
}
std::size_t Historical::MetadataGetBillableSize(const httplib::Params& params) {
  static const std::string kPath = ::BuildMetadataPath(".get_billable_size");
  const nlohmann::json json = client_.GetJson(kPath, params);
  if (!json.is_number_unsigned()) {
    throw JsonResponseError::TypeMismatch("Historical::MetadataGetBillableSize",
                                          "unsigned number", json);
  }
  return json;
}

static const std::string kMetadataGetCostEndpoint =
    "Historical::MetadataGetCost";

double Historical::MetadataGetCost(const std::string& dataset, UnixNanos start,
                                   UnixNanos end,
                                   const std::vector<std::string>& symbols,
                                   Schema schema) {
  return this->MetadataGetCost(dataset, start, end, symbols, schema,
                               FeedMode::HistoricalStreaming, kDefaultSTypeIn,
                               {});
}
double Historical::MetadataGetCost(const std::string& dataset,
                                   const std::string& start,
                                   const std::string& end,
                                   const std::vector<std::string>& symbols,
                                   Schema schema) {
  return this->MetadataGetCost(dataset, start, end, symbols, schema,
                               FeedMode::HistoricalStreaming, kDefaultSTypeIn,
                               {});
}
double Historical::MetadataGetCost(const std::string& dataset, UnixNanos start,
                                   UnixNanos end,
                                   const std::vector<std::string>& symbols,
                                   Schema schema, FeedMode mode, SType stype_in,
                                   std::size_t limit) {
  static const std::string kPath = ::BuildMetadataPath(".get_cost");
  httplib::Params params{
      {"dataset", dataset},
      {"start", ToString(start)},
      {"end", ToString(end)},
      {"symbols", JoinSymbolStrings(kMetadataGetCostEndpoint, symbols)},
      {"mode", ToString(mode)},
      {"schema", ToString(schema)},
      {"stype_in", ToString(stype_in)}};
  ::SetIfPositive(&params, "limit", limit);
  return this->MetadataGetCost(params);
}
double Historical::MetadataGetCost(const std::string& dataset,
                                   const std::string& start,
                                   const std::string& end,
                                   const std::vector<std::string>& symbols,
                                   Schema schema, FeedMode mode, SType stype_in,
                                   std::size_t limit) {
  static const std::string kPath = ::BuildMetadataPath(".get_cost");
  httplib::Params params{
      {"dataset", dataset},
      {"start", start},
      {"end", end},
      {"symbols", JoinSymbolStrings(kMetadataGetCostEndpoint, symbols)},
      {"mode", ToString(mode)},
      {"schema", ToString(schema)},
      {"stype_in", ToString(stype_in)}};
  ::SetIfPositive(&params, "limit", limit);
  return this->MetadataGetCost(params);
}
double Historical::MetadataGetCost(const HttplibParams& params) {
  static const std::string kPath = ::BuildMetadataPath(".get_cost");
  const nlohmann::json json = client_.GetJson(kPath, params);
  if (!json.is_number()) {
    throw JsonResponseError::TypeMismatch("Historical::MetadataGetCost",
                                          "number", json);
  }
  return json;
}

databento::SymbologyResolution Historical::SymbologyResolve(
    const std::string& dataset, const std::string& start_date,
    const std::string& end_date, const std::vector<std::string>& symbols,
    SType stype_in, SType stype_out) {
  return this->SymbologyResolve(dataset, start_date, end_date, symbols,
                                stype_in, stype_out, {});
}
databento::SymbologyResolution Historical::SymbologyResolve(
    const std::string& dataset, const std::string& start_date,
    const std::string& end_date, const std::vector<std::string>& symbols,
    SType stype_in, SType stype_out, const std::string& default_value) {
  static const std::string kEndpoint = "Historical::SymbologyResolve";
  static const std::string kPath = ::BuildSymbologyPath(".resolve");
  httplib::Params params{{"dataset", dataset},
                         {"start_date", start_date},
                         {"end_date", end_date},
                         {"symbols", JoinSymbolStrings(kEndpoint, symbols)},
                         {"stype_in", ToString(stype_in)},
                         {"stype_out", ToString(stype_out)}};
  ::SetIfNotEmpty(&params, "default_value", default_value);
  const nlohmann::json json = client_.GetJson(kPath, params);
  if (!json.is_object()) {
    throw JsonResponseError::TypeMismatch(kEndpoint, "object", json);
  }
  const auto& mappings_json = ::CheckedAt(kEndpoint, json, "result");
  const auto& partial_json = ::CheckedAt(kEndpoint, json, "partial");
  const auto& not_found_json = ::CheckedAt(kEndpoint, json, "not_found");
  SymbologyResolution res{};
  if (!mappings_json.is_object()) {
    throw JsonResponseError::TypeMismatch(kEndpoint, "mappings object",
                                          mappings_json);
  }
  res.mappings.reserve(mappings_json.size());
  for (const auto& mapping : mappings_json.items()) {
    const auto& mapping_json = mapping.value();
    if (!mapping_json.is_array()) {
      throw JsonResponseError::TypeMismatch(kEndpoint, "array", mapping.key(),
                                            mapping_json);
    }
    std::vector<StrMappingInterval> mapping_intervals;
    mapping_intervals.reserve(mapping_json.size());
    for (const auto& interval_json : mapping_json.items()) {
      mapping_intervals.emplace_back(StrMappingInterval{
          ::CheckedAt(kEndpoint, interval_json.value(), "d0"),
          ::CheckedAt(kEndpoint, interval_json.value(), "d1"),
          ::CheckedAt(kEndpoint, interval_json.value(), "s"),
      });
    }
    res.mappings.emplace(mapping.key(), std::move(mapping_intervals));
  }
  if (!partial_json.is_array()) {
    throw JsonResponseError::TypeMismatch(kEndpoint, "partial array",
                                          partial_json);
  }
  res.partial.reserve(partial_json.size());
  for (const auto& symbol : partial_json.items()) {
    if (!symbol.value().is_string()) {
      throw JsonResponseError::TypeMismatch(kEndpoint, "nested string",
                                            symbol.key(), symbol.value());
    }
    res.partial.emplace_back(symbol.value());
  }
  if (!not_found_json.is_array()) {
    throw JsonResponseError::TypeMismatch(kEndpoint, "not_found array",
                                          not_found_json);
  }
  res.not_found.reserve(not_found_json.size());
  for (const auto& symbol : not_found_json.items()) {
    if (!symbol.value().is_string()) {
      throw JsonResponseError::TypeMismatch(kEndpoint, "nested string",
                                            symbol.key(), symbol.value());
    }
    res.not_found.emplace_back(symbol.value());
  }
  return res;
}

static const std::string kTimeseriesGetRangeEndpoint =
    "Historical::TimeseriesGetRange";
static const std::string kTimeseriesGetRangePath =
    ::BuildTimeseriesPath(".get_range");

void Historical::TimeseriesGetRange(const std::string& dataset, UnixNanos start,
                                    UnixNanos end,
                                    const std::vector<std::string>& symbols,
                                    Schema schema,
                                    const RecordCallback& record_callback) {
  this->TimeseriesGetRange(dataset, start, end, symbols, schema,
                           kDefaultSTypeIn, kDefaultSTypeOut, {}, {},
                           record_callback);
}
void Historical::TimeseriesGetRange(const std::string& dataset,
                                    const std::string& start,
                                    const std::string& end,
                                    const std::vector<std::string>& symbols,
                                    Schema schema,
                                    const RecordCallback& record_callback) {
  this->TimeseriesGetRange(dataset, start, end, symbols, schema,
                           kDefaultSTypeIn, kDefaultSTypeOut, {}, {},
                           record_callback);
}
void Historical::TimeseriesGetRange(const std::string& dataset, UnixNanos start,
                                    UnixNanos end,
                                    const std::vector<std::string>& symbols,
                                    Schema schema, SType stype_in,
                                    SType stype_out, std::size_t limit,
                                    const MetadataCallback& metadata_callback,
                                    const RecordCallback& record_callback) {
  httplib::Params params{
      {"dataset", dataset},
      {"encoding", "dbn"},
      {"start", ToString(start)},
      {"end", ToString(end)},
      {"symbols", JoinSymbolStrings(kTimeseriesGetRangeEndpoint, symbols)},
      {"schema", ToString(schema)},
      {"stype_in", ToString(stype_in)},
      {"stype_out", ToString(stype_out)}};
  ::SetIfPositive(&params, "limit", limit);

  this->TimeseriesGetRange(params, metadata_callback, record_callback);
}
void Historical::TimeseriesGetRange(const std::string& dataset,
                                    const std::string& start,
                                    const std::string& end,
                                    const std::vector<std::string>& symbols,
                                    Schema schema, SType stype_in,
                                    SType stype_out, std::size_t limit,
                                    const MetadataCallback& metadata_callback,
                                    const RecordCallback& record_callback) {
  httplib::Params params{
      {"dataset", dataset},
      {"encoding", "dbn"},
      {"start", start},
      {"end", end},
      {"symbols", JoinSymbolStrings(kTimeseriesGetRangeEndpoint, symbols)},
      {"schema", ToString(schema)},
      {"stype_in", ToString(stype_in)},
      {"stype_out", ToString(stype_out)}};
  ::SetIfPositive(&params, "limit", limit);

  this->TimeseriesGetRange(params, metadata_callback, record_callback);
}
void Historical::TimeseriesGetRange(const HttplibParams& params,
                                    const MetadataCallback& metadata_callback,
                                    const RecordCallback& record_callback) {
  std::atomic<bool> should_continue{true};
  detail::SharedChannel channel;
  std::exception_ptr exception_ptr{};
  detail::ScopedThread stream{[this, &channel, &exception_ptr, &params,
                               &should_continue] {
    try {
      this->client_.GetRawStream(
          kTimeseriesGetRangePath, params,
          [channel, &should_continue](const char* data,
                                      std::size_t length) mutable {
            channel.Write(reinterpret_cast<const std::uint8_t*>(data), length);
            return should_continue.load();
          });
      channel.Finish();
    } catch (const std::exception&) {
      channel.Finish();
      // rethrowing here will cause the process to be terminated
      exception_ptr = std::current_exception();
    }
  }};
  try {
    DbnDecoder dbn_decoder{channel};
    Metadata metadata = dbn_decoder.ParseMetadata();
    const auto record_count = metadata.record_count;
    if (metadata_callback) {
      metadata_callback(std::move(metadata));
    }
    for (auto i = 0UL; i < record_count; ++i) {
      const bool should_stop =
          record_callback(dbn_decoder.ParseRecord()) == KeepGoing::Stop;
      if (should_stop) {
        should_continue = false;
        break;
      }
    }
  } catch (const std::exception& exc) {
    should_continue = false;
    // wait for thread to finish before checking for exceptions
    stream.Join();
    // check if there's an exception from stream thread. Thread safe because
    // `stream` thread has been joined
    if (exception_ptr) {
      std::rethrow_exception(exception_ptr);
    }
    // otherwise rethrow original exception
    throw;
  }
}

static const std::string kTimeseriesGetRangeToFileEndpoint =
    "Historical::TimeseriesGetRangeToFile";

databento::FileBento Historical::TimeseriesGetRangeToFile(
    const std::string& dataset, UnixNanos start, UnixNanos end,
    const std::vector<std::string>& symbols, Schema schema,
    const std::string& file_path) {
  return this->TimeseriesGetRangeToFile(dataset, start, end, symbols, schema,
                                        kDefaultSTypeIn, kDefaultSTypeOut, {},
                                        file_path);
}
databento::FileBento Historical::TimeseriesGetRangeToFile(
    const std::string& dataset, const std::string& start,
    const std::string& end, const std::vector<std::string>& symbols,
    Schema schema, const std::string& file_path) {
  return this->TimeseriesGetRangeToFile(dataset, start, end, symbols, schema,
                                        kDefaultSTypeIn, kDefaultSTypeOut, {},
                                        file_path);
}
databento::FileBento Historical::TimeseriesGetRangeToFile(
    const std::string& dataset, UnixNanos start, UnixNanos end,
    const std::vector<std::string>& symbols, Schema schema, SType stype_in,
    SType stype_out, std::size_t limit, const std::string& file_path) {
  httplib::Params params{
      {"dataset", dataset},
      {"encoding", "dbn"},
      {"start", ToString(start)},
      {"end", ToString(end)},
      {"symbols",
       JoinSymbolStrings(kTimeseriesGetRangeToFileEndpoint, symbols)},
      {"schema", ToString(schema)},
      {"stype_in", ToString(stype_in)},
      {"stype_out", ToString(stype_out)}};
  ::SetIfPositive(&params, "limit", limit);
  return this->TimeseriesGetRangeToFile(params, file_path);
}
databento::FileBento Historical::TimeseriesGetRangeToFile(
    const std::string& dataset, const std::string& start,
    const std::string& end, const std::vector<std::string>& symbols,
    Schema schema, SType stype_in, SType stype_out, std::size_t limit,
    const std::string& file_path) {
  httplib::Params params{
      {"dataset", dataset},
      {"encoding", "dbn"},
      {"start", start},
      {"end", end},
      {"symbols",
       JoinSymbolStrings(kTimeseriesGetRangeToFileEndpoint, symbols)},
      {"schema", ToString(schema)},
      {"stype_in", ToString(stype_in)},
      {"stype_out", ToString(stype_out)}};
  ::SetIfPositive(&params, "limit", limit);
  return this->TimeseriesGetRangeToFile(params, file_path);
}
databento::FileBento Historical::TimeseriesGetRangeToFile(
    const HttplibParams& params, const std::string& file_path) {
  {
    std::ofstream out_file{file_path, std::ios::binary};
    if (out_file.fail()) {
      throw InvalidArgumentError{kTimeseriesGetRangeEndpoint, "file_path",
                                 "Non-existent or invalid file"};
    }
    this->client_.GetRawStream(
        kTimeseriesGetRangePath, params,
        [&out_file](const char* data, std::size_t length) {
          out_file.write(data, static_cast<std::streamsize>(length));
          return true;
        });
  }  // close out_file
  return FileBento{file_path};
}

using databento::HistoricalBuilder;

HistoricalBuilder& HistoricalBuilder::SetKeyFromEnv() {
  char const* env_key = std::getenv("DATABENTO_API_KEY");
  if (env_key == nullptr) {
    throw Exception{
        "Expected environment variable DATABENTO_API_KEY to be set"};
  }
  return this->SetKey(env_key);
}

HistoricalBuilder& HistoricalBuilder::SetKey(std::string key) {
  key_ = std::move(key);
  return *this;
}

HistoricalBuilder& HistoricalBuilder::SetGateway(HistoricalGateway gateway) {
  gateway_ = gateway;
  return *this;
}

Historical HistoricalBuilder::Build() {
  if (key_.empty()) {
    throw Exception{"'key' is unset"};
  }
  return Historical{std::move(key_), gateway_};
}
