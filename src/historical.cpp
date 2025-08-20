#include "databento/historical.hpp"

#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif
#include <httplib.h>
#include <nlohmann/json.hpp>

#include <algorithm>  // find_if
#include <cstddef>    // size_t
#include <cstdlib>    // get_env
#include <filesystem>
#include <iterator>  // back_inserter
#include <sstream>
#include <string>
#include <system_error>
#include <utility>  // move

#include "databento/constants.hpp"
#include "databento/datetime.hpp"
#include "databento/dbn_file_store.hpp"
#include "databento/detail/dbn_buffer_decoder.hpp"
#include "databento/detail/json_helpers.hpp"
#include "databento/enums.hpp"
#include "databento/exceptions.hpp"  // Exception, JsonResponseError
#include "databento/file_stream.hpp"
#include "databento/log.hpp"
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

constexpr auto kDefaultSTypeIn = databento::SType::RawSymbol;
constexpr auto kDefaultEncoding = databento::Encoding::Dbn;
constexpr auto kDefaultCompression = databento::Compression::Zstd;
constexpr auto kDefaultSTypeOut = databento::SType::InstrumentId;

databento::BatchJob Parse(const std::string& endpoint, const nlohmann::json& json) {
  using databento::Compression;
  using databento::Delivery;
  using databento::Encoding;
  using databento::JobState;
  using databento::JsonResponseError;
  using databento::Schema;
  using databento::SplitDuration;
  using databento::SType;
  using databento::detail::CheckedAt;
  using databento::detail::FromCheckedAtString;
  using databento::detail::FromCheckedAtStringOrNull;
  using databento::detail::ParseAt;

  if (!json.is_object()) {
    throw JsonResponseError::TypeMismatch(endpoint, "object", json);
  }
  databento::BatchJob res;
  res.id = CheckedAt(endpoint, json, "id");
  res.user_id = ParseAt<std::string>(endpoint, json, "user_id");
  res.cost_usd = ParseAt<double>(endpoint, json, "cost_usd");
  res.dataset = ParseAt<std::string>(endpoint, json, "dataset");
  res.symbols = ParseAt<std::vector<std::string>>(endpoint, json, "symbols");
  res.stype_in = FromCheckedAtString<SType>(endpoint, json, "stype_in");
  res.stype_out = FromCheckedAtString<SType>(endpoint, json, "stype_out");
  res.schema = FromCheckedAtString<Schema>(endpoint, json, "schema");
  res.start = ParseAt<std::string>(endpoint, json, "start");
  res.end = ParseAt<std::string>(endpoint, json, "end");
  res.limit = ParseAt<std::uint64_t>(endpoint, json, "limit");
  res.encoding = FromCheckedAtString<Encoding>(endpoint, json, "encoding");
  res.compression = FromCheckedAtStringOrNull<Compression>(
      endpoint, json, "compression", Compression::None);
  res.pretty_px = ParseAt<bool>(endpoint, json, "pretty_px");
  res.pretty_ts = ParseAt<bool>(endpoint, json, "pretty_ts");
  res.map_symbols = ParseAt<bool>(endpoint, json, "map_symbols");
  res.split_duration = FromCheckedAtStringOrNull<SplitDuration>(
      endpoint, json, "split_duration", SplitDuration::None);
  res.split_size = ParseAt<std::uint64_t>(endpoint, json, "split_size");
  res.split_symbols = ParseAt<bool>(endpoint, json, "split_symbols");
  res.delivery = FromCheckedAtString<Delivery>(endpoint, json, "delivery");
  res.record_count = ParseAt<std::uint64_t>(endpoint, json, "record_count");
  res.billed_size = ParseAt<std::uint64_t>(endpoint, json, "billed_size");
  res.actual_size = ParseAt<std::uint64_t>(endpoint, json, "actual_size");
  res.package_size = ParseAt<std::uint64_t>(endpoint, json, "package_size");
  res.state = FromCheckedAtString<JobState>(endpoint, json, "state");
  res.ts_received = ParseAt<std::string>(endpoint, json, "ts_received");
  res.ts_queued = ParseAt<std::string>(endpoint, json, "ts_queued");
  res.ts_process_start = ParseAt<std::string>(endpoint, json, "ts_process_start");
  res.ts_process_done = ParseAt<std::string>(endpoint, json, "ts_process_done");
  res.ts_expiration = ParseAt<std::string>(endpoint, json, "ts_expiration");
  return res;
}

void TryCreateDir(const std::filesystem::path& dir_name) {
  using namespace std::string_literals;
  if (dir_name.empty()) {
    return;
  }
  std::error_code ec{};
  if (std::filesystem::create_directory(dir_name, ec) || !ec) {
    // Successfully created directory or it already exists
    return;
  }
  throw databento::Exception{"Unable to create directory "s +
                             dir_name.generic_string() + ": " + ec.message()};
}
}  // namespace

databento::HistoricalBuilder Historical::Builder() {
  return databento::HistoricalBuilder{};
}

Historical::Historical(ILogReceiver* log_receiver, std::string key,
                       HistoricalGateway gateway)
    : log_receiver_{log_receiver},
      key_{std::move(key)},
      gateway_{UrlFromGateway(gateway)},
      upgrade_policy_{VersionUpgradePolicy::UpgradeToV3},
      client_{log_receiver, key_, gateway_} {}

Historical::Historical(ILogReceiver* log_receiver, std::string key,
                       HistoricalGateway gateway, VersionUpgradePolicy upgrade_policy,
                       std::string user_agent_ext)
    : log_receiver_{log_receiver},
      key_{std::move(key)},
      gateway_{UrlFromGateway(gateway)},
      user_agent_ext_{std::move(user_agent_ext)},
      upgrade_policy_{upgrade_policy},
      client_{log_receiver, key_, gateway_} {}

Historical::Historical(ILogReceiver* log_receiver, std::string key, std::string gateway,
                       std::uint16_t port, VersionUpgradePolicy upgrade_policy,
                       std::string user_agent_ext)
    : log_receiver_{log_receiver},
      key_{std::move(key)},
      gateway_{std::move(gateway)},
      user_agent_ext_{std::move(user_agent_ext)},
      upgrade_policy_{upgrade_policy},
      client_{log_receiver, key_, gateway_, port} {}

static const std::string kBatchSubmitJobEndpoint = "Historical::BatchSubmitJob";

databento::BatchJob Historical::BatchSubmitJob(
    const std::string& dataset, const std::vector<std::string>& symbols, Schema schema,
    const DateTimeRange<UnixNanos>& datetime_range) {
  return this->BatchSubmitJob(dataset, symbols, schema, datetime_range,
                              kDefaultEncoding, kDefaultCompression, {}, {}, {}, {},
                              SplitDuration::Day, {}, Delivery::Download,
                              kDefaultSTypeIn, kDefaultSTypeOut, {});
}
databento::BatchJob Historical::BatchSubmitJob(
    const std::string& dataset, const std::vector<std::string>& symbols, Schema schema,
    const DateTimeRange<std::string>& datetime_range) {
  return this->BatchSubmitJob(dataset, symbols, schema, datetime_range,
                              kDefaultEncoding, kDefaultCompression, {}, {}, {}, {},
                              SplitDuration::Day, {}, Delivery::Download,
                              kDefaultSTypeIn, kDefaultSTypeOut, {});
}
databento::BatchJob Historical::BatchSubmitJob(
    const std::string& dataset, const std::vector<std::string>& symbols, Schema schema,
    const DateTimeRange<UnixNanos>& datetime_range, Encoding encoding,
    Compression compression, bool pretty_px, bool pretty_ts, bool map_symbols,
    bool split_symbols, SplitDuration split_duration, std::uint64_t split_size,
    Delivery delivery, SType stype_in, SType stype_out, std::uint64_t limit) {
  httplib::Params params{
      {"dataset", dataset},
      {"start", ToString(datetime_range.start)},
      {"symbols", JoinSymbolStrings(kBatchSubmitJobEndpoint, symbols)},
      {"schema", ToString(schema)},
      {"encoding", ToString(encoding)},
      {"compression", ToString(compression)},
      {"pretty_px", std::to_string(pretty_px)},
      {"pretty_ts", std::to_string(pretty_ts)},
      {"map_symbols", std::to_string(map_symbols)},
      {"split_symbols", std::to_string(split_symbols)},
      {"split_duration", ToString(split_duration)},
      {"delivery", ToString(delivery)},
      {"stype_in", ToString(stype_in)},
      {"stype_out", ToString(stype_out)}};
  detail::SetIfPositive(&params, "end", datetime_range.end);
  detail::SetIfPositive(&params, "split_size", split_size);
  detail::SetIfPositive(&params, "limit", limit);
  return this->BatchSubmitJob(params);
}
databento::BatchJob Historical::BatchSubmitJob(
    const std::string& dataset, const std::vector<std::string>& symbols, Schema schema,
    const DateTimeRange<std::string>& datetime_range, Encoding encoding,
    Compression compression, bool pretty_px, bool pretty_ts, bool map_symbols,
    bool split_symbols, SplitDuration split_duration, std::uint64_t split_size,
    Delivery delivery, SType stype_in, SType stype_out, std::uint64_t limit) {
  httplib::Params params{
      {"dataset", dataset},
      {"start", datetime_range.start},
      {"symbols", JoinSymbolStrings(kBatchSubmitJobEndpoint, symbols)},
      {"schema", ToString(schema)},
      {"encoding", ToString(encoding)},
      {"compression", ToString(compression)},
      {"pretty_px", std::to_string(pretty_px)},
      {"pretty_ts", std::to_string(pretty_ts)},
      {"map_symbols", std::to_string(map_symbols)},
      {"split_symbols", std::to_string(split_symbols)},
      {"split_duration", ToString(split_duration)},
      {"delivery", ToString(delivery)},
      {"stype_in", ToString(stype_in)},
      {"stype_out", ToString(stype_out)}};
  detail::SetIfNotEmpty(&params, "end", datetime_range.end);
  detail::SetIfPositive(&params, "split_size", split_size);
  detail::SetIfPositive(&params, "limit", limit);
  return this->BatchSubmitJob(params);
}
databento::BatchJob Historical::BatchSubmitJob(const httplib::Params& params) {
  static const std::string kPath = ::BuildBatchPath(".submit_job");
  const nlohmann::json json = client_.PostJson(kPath, params);
  return ::Parse("BatchSubmitJob", json);
}

std::vector<databento::BatchJob> Historical::BatchListJobs() {
  static const std::vector<JobState> kDefaultStates = {
      JobState::Queued, JobState::Processing, JobState::Done};
  return this->BatchListJobs(kDefaultStates, UnixNanos{});
}
std::vector<databento::BatchJob> Historical::BatchListJobs(
    const std::vector<databento::JobState>& states, UnixNanos since) {
  httplib::Params params;
  detail::SetIfNotEmpty(&params, "states", states);
  detail::SetIfPositive(&params, "since", since.time_since_epoch().count());
  return this->BatchListJobs(params);
}
std::vector<databento::BatchJob> Historical::BatchListJobs(
    const std::vector<databento::JobState>& states, const std::string& since) {
  httplib::Params params;
  detail::SetIfNotEmpty(&params, "states", states);
  detail::SetIfNotEmpty(&params, "since", since);
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
  std::transform(json.begin(), json.end(), std::back_inserter(jobs),
                 [](const nlohmann::json& item) { return ::Parse(kEndpoint, item); });
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
    file_desc.filename = detail::ParseAt<std::string>(kEndpoint, file_obj, "filename");
    file_desc.size = detail::ParseAt<std::uint64_t>(kEndpoint, file_obj, "size");
    file_desc.hash = detail::ParseAt<std::string>(kEndpoint, file_obj, "hash");
    const auto& url_obj = detail::CheckedAt(kEndpoint, file_obj, "urls");
    file_desc.https_url = detail::ParseAt<std::string>(kEndpoint, url_obj, "https");
    file_desc.ftp_url = detail::ParseAt<std::string>(kEndpoint, url_obj, "ftp");
    files.emplace_back(file_desc);
  }
  return files;
}

std::vector<std::filesystem::path> Historical::BatchDownload(
    const std::filesystem::path& output_dir, const std::string& job_id) {
  TryCreateDir(output_dir);
  const std::filesystem::path job_dir = output_dir / job_id;
  TryCreateDir(job_dir);
  const auto file_descs = BatchListFiles(job_id);
  std::vector<std::filesystem::path> paths;
  for (const auto& file_desc : file_descs) {
    std::filesystem::path output_path = job_dir / file_desc.filename;
    DownloadFile(file_desc.https_url, output_path);
    paths.emplace_back(std::move(output_path));
  }
  return paths;
}
std::filesystem::path Historical::BatchDownload(
    const std::filesystem::path& output_dir, const std::string& job_id,
    const std::string& filename_to_download) {
  TryCreateDir(output_dir);
  const std::filesystem::path job_dir = output_dir / job_id;
  TryCreateDir(job_dir);
  const auto file_descs = BatchListFiles(job_id);
  const auto file_desc_it =
      std::find_if(file_descs.begin(), file_descs.end(),
                   [&filename_to_download](const BatchFileDesc& file_desc) {
                     return file_desc.filename == filename_to_download;
                   });
  if (file_desc_it == file_descs.end()) {
    throw InvalidArgumentError{"Historical::BatchDownload", "filename_to_download",
                               "Filename not found for batch job " + job_id};
  }
  std::filesystem::path output_path = job_dir / file_desc_it->filename;
  DownloadFile(file_desc_it->https_url, output_path);
  return output_path;
}

void Historical::DownloadFile(const std::string& url,
                              const std::filesystem::path& output_path) {
  static const std::string kMethod = "Historical::DownloadFile";
  // extract path from URL
  const auto protocol_divider = url.find("://");
  std::string path;

  if (protocol_divider == std::string::npos) {
    const auto slash = url.find_first_of('/');
    if (slash == std::string::npos) {
      throw InvalidArgumentError{kMethod, "url", "No slashes"};
    }
    path = url.substr(slash);
  } else {
    const auto slash = url.find('/', protocol_divider + 3);
    if (slash == std::string::npos) {
      throw InvalidArgumentError{kMethod, "url", "No slashes"};
    }
    path = url.substr(slash);
  }
  std::ostringstream ss;
  ss << '[' << kMethod << "] Downloading batch file " << path << " to " << output_path;
  log_receiver_->Receive(LogLevel::Info, ss.str());

  OutFileStream out_file{output_path};
  this->client_.GetRawStream(
      path, {}, [&out_file](const char* data, std::size_t length) {
        out_file.WriteAll(reinterpret_cast<const std::byte*>(data), length);
        return true;
      });

  if (log_receiver_->ShouldLog(LogLevel::Debug)) {
    ss.str("");
    ss << '[' << kMethod << ']' << " Completed download of " << path;
    log_receiver_->Receive(LogLevel::Debug, ss.str());
  }
}

std::vector<databento::PublisherDetail> Historical::MetadataListPublishers() {
  static const std::string kEndpoint = "Historical::MetadataListPublishers";
  static const std::string kPath = ::BuildMetadataPath(".list_publishers");
  const nlohmann::json json = client_.GetJson(kPath, httplib::Params{});
  if (!json.is_array()) {
    throw JsonResponseError::TypeMismatch(kEndpoint, "array", json);
  }
  std::vector<PublisherDetail> publisher_details;
  for (const auto& detail_json : json) {
    if (!detail_json.is_object()) {
      throw JsonResponseError::TypeMismatch(kEndpoint, "object", detail_json);
    }
    const auto id =
        detail::ParseAt<std::uint16_t>(kEndpoint, detail_json, "publisher_id");
    auto dataset = detail::ParseAt<std::string>(kEndpoint, detail_json, "dataset");
    auto venue = detail::ParseAt<std::string>(kEndpoint, detail_json, "venue");
    auto description =
        detail::ParseAt<std::string>(kEndpoint, detail_json, "description");
    publisher_details.emplace_back(PublisherDetail{
        id, std::move(dataset), std::move(venue), std::move(description)});
  }
  return publisher_details;
}

std::vector<std::string> Historical::MetadataListDatasets() {
  return this->MetadataListDatasets(DateRange{{}});
}
std::vector<std::string> Historical::MetadataListDatasets(const DateRange& date_range) {
  static const std::string kEndpoint = "Historical::MetadataListDatasets";
  static const std::string kPath = ::BuildMetadataPath(".list_datasets");
  httplib::Params params{};
  detail::SetIfNotEmpty(&params, "start_date", date_range.start);
  detail::SetIfNotEmpty(&params, "end_date", date_range.end);
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
  static const std::string kEndpoint = "Historical::MetadataListSchemas";
  static const std::string kPath = ::BuildMetadataPath(".list_schemas");
  const nlohmann::json json = client_.GetJson(kPath, {{"dataset", dataset}});
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

std::vector<databento::FieldDetail> Historical::MetadataListFields(Encoding encoding,
                                                                   Schema schema) {
  static const std::string kEndpoint = "Historical::MetadataListFields";
  static const std::string kPath = ::BuildMetadataPath(".list_fields");
  const nlohmann::json json = client_.GetJson(
      kPath, {{"encoding", ToString(encoding)}, {"schema", ToString(schema)}});
  if (!json.is_array()) {
    throw JsonResponseError::TypeMismatch(kEndpoint, "array", json);
  }
  std::vector<FieldDetail> field_details;
  for (const auto& detail_json : json) {
    if (!detail_json.is_object()) {
      throw JsonResponseError::TypeMismatch(kEndpoint, "object", detail_json);
    }
    auto name = detail::ParseAt<std::string>(kEndpoint, detail_json, "name");
    auto type = detail::ParseAt<std::string>(kEndpoint, detail_json, "type");
    field_details.emplace_back(FieldDetail{std::move(name), std::move(type)});
  }
  return field_details;
}

std::vector<databento::UnitPricesForMode> Historical::MetadataListUnitPrices(
    const std::string& dataset) {
  static const std::string kEndpoint = "Historical::MetadataListUnitPrices";
  static const std::string kPath = ::BuildMetadataPath(".list_unit_prices");
  const nlohmann::json json =
      client_.GetJson(kPath, httplib::Params{{"dataset", dataset}});
  if (!json.is_array()) {
    throw JsonResponseError::TypeMismatch(kEndpoint, "array", json);
  }
  std::vector<UnitPricesForMode> res;
  for (const auto& mode_json : json) {
    if (!mode_json.is_object()) {
      throw JsonResponseError::TypeMismatch(kEndpoint, "object", mode_json);
    }
    const auto mode =
        detail::FromCheckedAtString<FeedMode>(kEndpoint, mode_json, "mode");
    std::map<Schema, double> unit_prices;
    const auto unit_prices_json =
        detail::CheckedAt(kEndpoint, mode_json, "unit_prices");
    if (!unit_prices_json.is_object()) {
      throw JsonResponseError::TypeMismatch(kEndpoint, "nested object",
                                            unit_prices_json);
    }
    for (const auto& schema_json : unit_prices_json.items()) {
      if (!schema_json.value().is_number()) {
        throw JsonResponseError::TypeMismatch(kEndpoint, "nested number",
                                              schema_json.key(), schema_json.value());
      }
      unit_prices.emplace(FromString<Schema>(schema_json.key()), schema_json.value());
    }
    res.emplace_back(UnitPricesForMode{mode, std::move(unit_prices)});
  }
  return res;
}

std::vector<databento::DatasetConditionDetail> Historical::MetadataGetDatasetCondition(
    const std::string& dataset) {
  return MetadataGetDatasetCondition(httplib::Params{{"dataset", dataset}});
}

std::vector<databento::DatasetConditionDetail> Historical::MetadataGetDatasetCondition(
    const std::string& dataset, const DateRange& date_range) {
  httplib::Params params{{"dataset", dataset}, {"start_date", date_range.start}};
  detail::SetIfNotEmpty(&params, "end_date", date_range.end);
  return MetadataGetDatasetCondition(params);
}

std::vector<databento::DatasetConditionDetail> Historical::MetadataGetDatasetCondition(
    const httplib::Params& params) {
  static const std::string kEndpoint = "Historical::MetadataGetDatasetCondition";
  static const std::string kPath = ::BuildMetadataPath(".get_dataset_condition");
  const nlohmann::json json = client_.GetJson(kPath, params);
  if (!json.is_array()) {
    throw JsonResponseError::TypeMismatch(kEndpoint, "array", json);
  }
  std::vector<DatasetConditionDetail> details;
  details.reserve(json.size());
  for (const auto& detail_json : json) {
    if (!detail_json.is_object()) {
      throw JsonResponseError::TypeMismatch(kEndpoint, "object", detail_json);
    }
    auto date = detail::ParseAt<std::string>(kEndpoint, detail_json, "date");
    const auto condition = detail::FromCheckedAtString<DatasetCondition>(
        kEndpoint, detail_json, "condition");
    auto last_modified_date = detail::ParseAt<std::optional<std::string>>(
        kEndpoint, detail_json, "last_modified_date");
    details.emplace_back(DatasetConditionDetail{std::move(date), condition,
                                                std::move(last_modified_date)});
  }
  return details;
}

databento::DatasetRange Historical::MetadataGetDatasetRange(
    const std::string& dataset) {
  static const std::string kEndpoint = "Historical::GetDatasetRange";
  static const std::string kPath = ::BuildMetadataPath(".get_dataset_range");
  const nlohmann::json json = client_.GetJson(kPath, {{"dataset", dataset}});
  if (!json.is_object()) {
    throw JsonResponseError::TypeMismatch(kEndpoint, "object", json);
  }
  const auto& schema_json = detail::CheckedAt(kEndpoint, json, "schema");
  if (!schema_json.is_object()) {
    throw JsonResponseError::TypeMismatch(kEndpoint, "schema object", json);
  }
  std::map<Schema, DateTimeRange<std::string>> range_by_schema;
  for (const auto& schema_item : schema_json.items()) {
    if (!schema_item.value().is_object()) {
      throw JsonResponseError::TypeMismatch(kEndpoint, "nested schema object", json);
    }
    auto start = detail::ParseAt<std::string>(kEndpoint, schema_item.value(), "start");
    auto end = detail::ParseAt<std::string>(kEndpoint, schema_item.value(), "end");
    range_by_schema.emplace(
        FromString<Schema>(schema_item.key()),
        DateTimeRange<std::string>{std::move(start), std::move(end)});
  }
  return DatasetRange{detail::ParseAt<std::string>(kEndpoint, json, "start"),
                      detail::ParseAt<std::string>(kEndpoint, json, "end"),
                      std::move(range_by_schema)};
}

static const std::string kMetadataGetRecordCountEndpoint =
    "Historical::MetadataGetRecordCount";

std::uint64_t Historical::MetadataGetRecordCount(
    const std::string& dataset, const DateTimeRange<UnixNanos>& datetime_range,
    const std::vector<std::string>& symbols, Schema schema) {
  return this->MetadataGetRecordCount(dataset, datetime_range, symbols, schema,
                                      kDefaultSTypeIn, {});
}
std::uint64_t Historical::MetadataGetRecordCount(
    const std::string& dataset, const DateTimeRange<std::string>& datetime_range,
    const std::vector<std::string>& symbols, Schema schema) {
  return this->MetadataGetRecordCount(dataset, datetime_range, symbols, schema,
                                      kDefaultSTypeIn, {});
}
std::uint64_t Historical::MetadataGetRecordCount(
    const std::string& dataset, const DateTimeRange<UnixNanos>& datetime_range,
    const std::vector<std::string>& symbols, Schema schema, SType stype_in,
    std::uint64_t limit) {
  httplib::Params params{
      {"dataset", dataset},
      {"start", ToString(datetime_range.start)},
      {"symbols", JoinSymbolStrings(kMetadataGetRecordCountEndpoint, symbols)},
      {"schema", ToString(schema)},
      {"stype_in", ToString(stype_in)}};
  detail::SetIfPositive(&params, "end", datetime_range.end);
  detail::SetIfPositive(&params, "limit", limit);
  return this->MetadataGetRecordCount(params);
}
std::uint64_t Historical::MetadataGetRecordCount(
    const std::string& dataset, const DateTimeRange<std::string>& datetime_range,
    const std::vector<std::string>& symbols, Schema schema, SType stype_in,
    std::uint64_t limit) {
  httplib::Params params{
      {"dataset", dataset},
      {"start", datetime_range.start},
      {"symbols", JoinSymbolStrings(kMetadataGetRecordCountEndpoint, symbols)},
      {"schema", ToString(schema)},
      {"stype_in", ToString(stype_in)}};
  detail::SetIfNotEmpty(&params, "end", datetime_range.end);
  detail::SetIfPositive(&params, "limit", limit);
  return this->MetadataGetRecordCount(params);
}
std::uint64_t Historical::MetadataGetRecordCount(const httplib::Params& params) {
  static const std::string kPath = ::BuildMetadataPath(".get_record_count");
  const nlohmann::json json = client_.PostJson(kPath, params);
  if (!json.is_number_unsigned()) {
    throw JsonResponseError::TypeMismatch("Historical::MetadataGetRecordCount",
                                          "unsigned number", json);
  }
  return json;
}

static const std::string kMetadataGetBillableSizeEndpoint =
    "Historical::MetadataGetBillableSize";

std::uint64_t Historical::MetadataGetBillableSize(
    const std::string& dataset, const DateTimeRange<UnixNanos>& datetime_range,
    const std::vector<std::string>& symbols, Schema schema) {
  return this->MetadataGetBillableSize(dataset, datetime_range, symbols, schema,
                                       kDefaultSTypeIn, {});
}
std::uint64_t Historical::MetadataGetBillableSize(
    const std::string& dataset, const DateTimeRange<std::string>& datetime_range,
    const std::vector<std::string>& symbols, Schema schema) {
  return this->MetadataGetBillableSize(dataset, datetime_range, symbols, schema,
                                       kDefaultSTypeIn, {});
}
std::uint64_t Historical::MetadataGetBillableSize(

    const std::string& dataset, const DateTimeRange<UnixNanos>& datetime_range,
    const std::vector<std::string>& symbols, Schema schema, SType stype_in,
    std::uint64_t limit) {
  httplib::Params params{
      {"dataset", dataset},
      {"start", ToString(datetime_range.start)},
      {"symbols", JoinSymbolStrings(kMetadataGetBillableSizeEndpoint, symbols)},
      {"schema", ToString(schema)},
      {"stype_in", ToString(stype_in)}};
  detail::SetIfPositive(&params, "end", datetime_range.end);
  detail::SetIfPositive(&params, "limit", limit);
  return this->MetadataGetBillableSize(params);
}
std::uint64_t Historical::MetadataGetBillableSize(
    const std::string& dataset, const DateTimeRange<std::string>& datetime_range,
    const std::vector<std::string>& symbols, Schema schema, SType stype_in,
    std::uint64_t limit) {
  httplib::Params params{
      {"dataset", dataset},
      {"start", datetime_range.start},
      {"symbols", JoinSymbolStrings(kMetadataGetBillableSizeEndpoint, symbols)},
      {"schema", ToString(schema)},
      {"stype_in", ToString(stype_in)}};
  detail::SetIfNotEmpty(&params, "end", datetime_range.end);
  detail::SetIfPositive(&params, "limit", limit);
  return this->MetadataGetBillableSize(params);
}
std::uint64_t Historical::MetadataGetBillableSize(const httplib::Params& params) {
  static const std::string kPath = ::BuildMetadataPath(".get_billable_size");
  const nlohmann::json json = client_.PostJson(kPath, params);
  if (!json.is_number_unsigned()) {
    throw JsonResponseError::TypeMismatch("Historical::MetadataGetBillableSize",
                                          "unsigned number", json);
  }
  return json;
}

static const std::string kMetadataGetCostEndpoint = "Historical::MetadataGetCost";

double Historical::MetadataGetCost(const std::string& dataset,
                                   const DateTimeRange<UnixNanos>& datetime_range,
                                   const std::vector<std::string>& symbols,
                                   Schema schema) {
  return this->MetadataGetCost(dataset, datetime_range, symbols, schema,
                               FeedMode::HistoricalStreaming, kDefaultSTypeIn, {});
}
double Historical::MetadataGetCost(const std::string& dataset,
                                   const DateTimeRange<std::string>& datetime_range,
                                   const std::vector<std::string>& symbols,
                                   Schema schema) {
  return this->MetadataGetCost(dataset, datetime_range, symbols, schema,
                               FeedMode::HistoricalStreaming, kDefaultSTypeIn, {});
}
double Historical::MetadataGetCost(const std::string& dataset,
                                   const DateTimeRange<UnixNanos>& datetime_range,
                                   const std::vector<std::string>& symbols,
                                   Schema schema, FeedMode mode, SType stype_in,
                                   std::uint64_t limit) {
  httplib::Params params{
      {"dataset", dataset},
      {"start", ToString(datetime_range.start)},
      {"symbols", JoinSymbolStrings(kMetadataGetCostEndpoint, symbols)},
      {"mode", ToString(mode)},
      {"schema", ToString(schema)},
      {"stype_in", ToString(stype_in)}};
  detail::SetIfPositive(&params, "end", datetime_range.end);
  detail::SetIfPositive(&params, "limit", limit);
  return this->MetadataGetCost(params);
}
double Historical::MetadataGetCost(const std::string& dataset,
                                   const DateTimeRange<std::string>& datetime_range,
                                   const std::vector<std::string>& symbols,
                                   Schema schema, FeedMode mode, SType stype_in,
                                   std::uint64_t limit) {
  httplib::Params params{
      {"dataset", dataset},
      {"start", datetime_range.start},
      {"symbols", JoinSymbolStrings(kMetadataGetCostEndpoint, symbols)},
      {"mode", ToString(mode)},
      {"schema", ToString(schema)},
      {"stype_in", ToString(stype_in)}};
  detail::SetIfNotEmpty(&params, "end", datetime_range.end);
  detail::SetIfPositive(&params, "limit", limit);
  return this->MetadataGetCost(params);
}
double Historical::MetadataGetCost(const HttplibParams& params) {
  static const std::string kPath = ::BuildMetadataPath(".get_cost");
  const nlohmann::json json = client_.PostJson(kPath, params);
  if (!json.is_number()) {
    throw JsonResponseError::TypeMismatch("Historical::MetadataGetCost", "number",
                                          json);
  }
  return json;
}

databento::SymbologyResolution Historical::SymbologyResolve(
    const std::string& dataset, const std::vector<std::string>& symbols, SType stype_in,
    SType stype_out, const DateRange& date_range) {
  static const std::string kEndpoint = "Historical::SymbologyResolve";
  static const std::string kPath = ::BuildSymbologyPath(".resolve");
  httplib::Params params{{"dataset", dataset},
                         {"start_date", date_range.start},
                         {"symbols", JoinSymbolStrings(kEndpoint, symbols)},
                         {"stype_in", ToString(stype_in)},
                         {"stype_out", ToString(stype_out)}};
  detail::SetIfNotEmpty(&params, "end_date", date_range.end);
  const nlohmann::json json = client_.PostJson(kPath, params);
  if (!json.is_object()) {
    throw JsonResponseError::TypeMismatch(kEndpoint, "object", json);
  }
  const auto& mappings_json = detail::CheckedAt(kEndpoint, json, "result");
  const auto& partial_json = detail::CheckedAt(kEndpoint, json, "partial");
  const auto& not_found_json = detail::CheckedAt(kEndpoint, json, "not_found");
  SymbologyResolution res{{}, {}, {}, stype_in, stype_out};
  if (!mappings_json.is_object()) {
    throw JsonResponseError::TypeMismatch(kEndpoint, "mappings object", mappings_json);
  }
  res.mappings.reserve(mappings_json.size());
  for (const auto& mapping : mappings_json.items()) {
    const auto& mapping_json = mapping.value();
    if (!mapping_json.is_array()) {
      throw JsonResponseError::TypeMismatch(kEndpoint, "array", mapping.key(),
                                            mapping_json);
    }
    std::vector<MappingInterval> mapping_intervals;
    std::transform(
        mapping_json.begin(), mapping_json.end(), std::back_inserter(mapping_intervals),
        [](const nlohmann::json& interval_json) {
          return MappingInterval{
              detail::ParseAt<date::year_month_day>(kEndpoint, interval_json, "d0"),
              detail::ParseAt<date::year_month_day>(kEndpoint, interval_json, "d1"),
              detail::CheckedAt(kEndpoint, interval_json, "s"),
          };
        });
    res.mappings.emplace(mapping.key(), std::move(mapping_intervals));
  }
  if (!partial_json.is_array()) {
    throw JsonResponseError::TypeMismatch(kEndpoint, "partial array", partial_json);
  }
  res.partial.reserve(partial_json.size());
  for (const auto& symbol : partial_json.items()) {
    if (!symbol.value().is_string()) {
      throw JsonResponseError::TypeMismatch(kEndpoint, "nested string", symbol.key(),
                                            symbol.value());
    }
    res.partial.emplace_back(symbol.value());
  }
  if (!not_found_json.is_array()) {
    throw JsonResponseError::TypeMismatch(kEndpoint, "not_found array", not_found_json);
  }
  res.not_found.reserve(not_found_json.size());
  for (const auto& symbol : not_found_json.items()) {
    if (!symbol.value().is_string()) {
      throw JsonResponseError::TypeMismatch(kEndpoint, "nested string", symbol.key(),
                                            symbol.value());
    }
    res.not_found.emplace_back(symbol.value());
  }
  return res;
}

static const std::string kTimeseriesGetRangeEndpoint = "Historical::TimeseriesGetRange";
static const std::string kTimeseriesGetRangePath = ::BuildTimeseriesPath(".get_range");

void Historical::TimeseriesGetRange(const std::string& dataset,
                                    const DateTimeRange<UnixNanos>& datetime_range,
                                    const std::vector<std::string>& symbols,
                                    Schema schema,
                                    const RecordCallback& record_callback) {
  this->TimeseriesGetRange(dataset, datetime_range, symbols, schema, kDefaultSTypeIn,
                           kDefaultSTypeOut, {}, {}, record_callback);
}
void Historical::TimeseriesGetRange(const std::string& dataset,
                                    const DateTimeRange<std::string>& datetime_range,
                                    const std::vector<std::string>& symbols,
                                    Schema schema,
                                    const RecordCallback& record_callback) {
  this->TimeseriesGetRange(dataset, datetime_range, symbols, schema, kDefaultSTypeIn,
                           kDefaultSTypeOut, {}, {}, record_callback);
}
void Historical::TimeseriesGetRange(const std::string& dataset,
                                    const DateTimeRange<UnixNanos>& datetime_range,
                                    const std::vector<std::string>& symbols,
                                    Schema schema, SType stype_in, SType stype_out,
                                    std::uint64_t limit,
                                    const MetadataCallback& metadata_callback,
                                    const RecordCallback& record_callback) {
  httplib::Params params{
      {"dataset", dataset},
      {"encoding", "dbn"},
      {"compression", "zstd"},
      {"start", ToString(datetime_range.start)},
      {"symbols", JoinSymbolStrings(kTimeseriesGetRangeEndpoint, symbols)},
      {"schema", ToString(schema)},
      {"stype_in", ToString(stype_in)},
      {"stype_out", ToString(stype_out)}};
  detail::SetIfPositive(&params, "end", datetime_range.end);
  detail::SetIfPositive(&params, "limit", limit);

  this->TimeseriesGetRange(params, metadata_callback, record_callback);
}
void Historical::TimeseriesGetRange(const std::string& dataset,
                                    const DateTimeRange<std::string>& datetime_range,
                                    const std::vector<std::string>& symbols,
                                    Schema schema, SType stype_in, SType stype_out,
                                    std::uint64_t limit,
                                    const MetadataCallback& metadata_callback,
                                    const RecordCallback& record_callback) {
  httplib::Params params{
      {"dataset", dataset},
      {"encoding", "dbn"},
      {"compression", "zstd"},
      {"start", datetime_range.start},
      {"symbols", JoinSymbolStrings(kTimeseriesGetRangeEndpoint, symbols)},
      {"schema", ToString(schema)},
      {"stype_in", ToString(stype_in)},
      {"stype_out", ToString(stype_out)}};
  detail::SetIfNotEmpty(&params, "end", datetime_range.end);
  detail::SetIfPositive(&params, "limit", limit);

  this->TimeseriesGetRange(params, metadata_callback, record_callback);
}

enum class DecoderState : std::uint8_t {
  Init,
  Metadata,
  Records,
};
void Historical::TimeseriesGetRange(const HttplibParams& params,
                                    const MetadataCallback& metadata_callback,
                                    const RecordCallback& record_callback) {
  detail::DbnBufferDecoder decoder{upgrade_policy_, metadata_callback, record_callback};

  bool early_exit = false;
  this->client_.PostRawStream(
      kTimeseriesGetRangePath, params,
      [&decoder, &early_exit](const char* data, std::size_t length) mutable {
        if (decoder.Process(data, length) == KeepGoing::Continue) {
          return true;
        }
        early_exit = true;
        return false;
      });
  if (!early_exit && decoder.UnreadBytes() > 0) {
    std::ostringstream ss;
    ss << "[Historical::TimeseriesGetRange] Partial or incomplete record "
          "remaining of "
       << decoder.UnreadBytes() << " bytes";
    log_receiver_->Receive(LogLevel::Warning, ss.str());
  }
}

static const std::string kTimeseriesGetRangeToFileEndpoint =
    "Historical::TimeseriesGetRangeToFile";

databento::DbnFileStore Historical::TimeseriesGetRangeToFile(
    const std::string& dataset, const DateTimeRange<UnixNanos>& datetime_range,
    const std::vector<std::string>& symbols, Schema schema,
    const std::filesystem::path& file_path) {
  return this->TimeseriesGetRangeToFile(dataset, datetime_range, symbols, schema,
                                        kDefaultSTypeIn, kDefaultSTypeOut, {},
                                        file_path);
}
databento::DbnFileStore Historical::TimeseriesGetRangeToFile(
    const std::string& dataset, const DateTimeRange<std::string>& datetime_range,
    const std::vector<std::string>& symbols, Schema schema,
    const std::filesystem::path& file_path) {
  return this->TimeseriesGetRangeToFile(dataset, datetime_range, symbols, schema,
                                        kDefaultSTypeIn, kDefaultSTypeOut, {},
                                        file_path);
}
databento::DbnFileStore Historical::TimeseriesGetRangeToFile(
    const std::string& dataset, const DateTimeRange<UnixNanos>& datetime_range,
    const std::vector<std::string>& symbols, Schema schema, SType stype_in,
    SType stype_out, std::uint64_t limit, const std::filesystem::path& file_path) {
  httplib::Params params{
      {"dataset", dataset},
      {"encoding", "dbn"},
      {"compression", "zstd"},
      {"start", ToString(datetime_range.start)},
      {"symbols", JoinSymbolStrings(kTimeseriesGetRangeToFileEndpoint, symbols)},
      {"schema", ToString(schema)},
      {"stype_in", ToString(stype_in)},
      {"stype_out", ToString(stype_out)}};
  detail::SetIfPositive(&params, "end", datetime_range.end);
  detail::SetIfPositive(&params, "limit", limit);
  return this->TimeseriesGetRangeToFile(params, file_path);
}
databento::DbnFileStore Historical::TimeseriesGetRangeToFile(
    const std::string& dataset, const DateTimeRange<std::string>& datetime_range,
    const std::vector<std::string>& symbols, Schema schema, SType stype_in,
    SType stype_out, std::uint64_t limit, const std::filesystem::path& file_path) {
  httplib::Params params{
      {"dataset", dataset},
      {"encoding", "dbn"},
      {"compression", "zstd"},
      {"start", datetime_range.start},
      {"symbols", JoinSymbolStrings(kTimeseriesGetRangeToFileEndpoint, symbols)},
      {"schema", ToString(schema)},
      {"stype_in", ToString(stype_in)},
      {"stype_out", ToString(stype_out)}};
  detail::SetIfNotEmpty(&params, "end", datetime_range.end);
  detail::SetIfPositive(&params, "limit", limit);
  return this->TimeseriesGetRangeToFile(params, file_path);
}
databento::DbnFileStore Historical::TimeseriesGetRangeToFile(
    const HttplibParams& params, const std::filesystem::path& file_path) {
  {
    OutFileStream out_file{file_path};
    this->client_.PostRawStream(kTimeseriesGetRangePath, params,
                                [&out_file](const char* data, std::size_t length) {
                                  out_file.WriteAll(
                                      reinterpret_cast<const std::byte*>(data), length);
                                  return true;
                                });
  }  // Flush out_file
  return DbnFileStore{log_receiver_, file_path, upgrade_policy_};
}

using databento::HistoricalBuilder;

HistoricalBuilder& HistoricalBuilder::SetKeyFromEnv() {
  char const* env_key = std::getenv("DATABENTO_API_KEY");
  if (env_key == nullptr) {
    throw Exception{"Expected environment variable DATABENTO_API_KEY to be set"};
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

HistoricalBuilder& HistoricalBuilder::SetLogReceiver(ILogReceiver* log_receiver) {
  log_receiver_ = log_receiver;
  return *this;
}

HistoricalBuilder& HistoricalBuilder::SetUpgradePolicy(
    VersionUpgradePolicy upgrade_policy) {
  upgrade_policy_ = upgrade_policy;
  return *this;
}

HistoricalBuilder& HistoricalBuilder::SetAddress(std::string gateway,
                                                 std::uint16_t port) {
  gateway_override_ = std::move(gateway);
  port_ = port;
  return *this;
}

HistoricalBuilder& HistoricalBuilder::ExtendUserAgent(std::string extension) {
  user_agent_ext_ = std::move(extension);
  return *this;
}

Historical HistoricalBuilder::Build() {
  if (key_.empty()) {
    throw Exception{"'key' is unset"};
  }
  if (log_receiver_ == nullptr) {
    log_receiver_ = databento::ILogReceiver::Default();
  }
  if (gateway_override_.empty()) {
    return Historical{log_receiver_, key_, gateway_, upgrade_policy_, user_agent_ext_};
  }
  return Historical{log_receiver_,   key_,           gateway_override_, port_,
                    upgrade_policy_, user_agent_ext_};
}
