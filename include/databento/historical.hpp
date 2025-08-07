#pragma once

#include <cstdint>
#include <filesystem>
#include <map>  // multimap
#include <string>
#include <vector>

#include "databento/batch.hpp"     // BatchJob
#include "databento/datetime.hpp"  // DateRange, DateTimeRange, UnixNanos
#include "databento/dbn_file_store.hpp"
#include "databento/detail/http_client.hpp"  // HttpClient
#include "databento/enums.hpp"  // BatchState, Delivery, DurationInterval, Schema, SType, VersionUpgradePolicy
#include "databento/metadata.hpp"  // DatasetConditionDetail, DatasetRange, FieldDetail, PublisherDetail, UnitPricesForMode
#include "databento/symbology.hpp"   // SymbologyResolution
#include "databento/timeseries.hpp"  // KeepGoing, MetadataCallback, RecordCallback

namespace databento {
// Forward declarations
class HistoricalBuilder;
class ILogReceiver;

// A client for interfacing with Databento's historical market data API.
class Historical {
 public:
  static HistoricalBuilder Builder();

  // WARNING: Will be deprecated in the future in favor of the builder
  Historical(ILogReceiver* log_receiver, std::string key, HistoricalGateway gateway);

  /*
   * Getters
   */

  const std::string& Key() const { return key_; };
  const std::string& Gateway() const { return gateway_; }

  /*
   * Batch API
   */

  // Submits a new batch job and returns a description of the job.
  //
  // WARNING: Calling this method will incur a cost.
  BatchJob BatchSubmitJob(const std::string& dataset,
                          const std::vector<std::string>& symbols, Schema schema,
                          const DateTimeRange<UnixNanos>& datetime_range);
  BatchJob BatchSubmitJob(const std::string& dataset,
                          const std::vector<std::string>& symbols, Schema schema,
                          const DateTimeRange<std::string>& datetime_range);
  BatchJob BatchSubmitJob(const std::string& dataset,
                          const std::vector<std::string>& symbols, Schema schema,
                          const DateTimeRange<UnixNanos>& datetime_range,
                          Encoding encoding, Compression compression, bool pretty_px,
                          bool pretty_ts, bool map_symbols, bool split_symbols,
                          SplitDuration split_duration, std::uint64_t split_size,
                          Delivery delivery, SType stype_in, SType stype_out,
                          std::uint64_t limit);
  BatchJob BatchSubmitJob(const std::string& dataset,
                          const std::vector<std::string>& symbols, Schema schema,
                          const DateTimeRange<std::string>& datetime_range,
                          Encoding encoding, Compression compression, bool pretty_px,
                          bool pretty_ts, bool map_symbols, bool split_symbols,
                          SplitDuration split_duration, std::uint64_t split_size,
                          Delivery delivery, SType stype_in, SType stype_out,
                          std::uint64_t limit);
  // Lists previous batch jobs.
  std::vector<BatchJob> BatchListJobs();
  std::vector<BatchJob> BatchListJobs(const std::vector<JobState>& states,
                                      UnixNanos since);
  std::vector<BatchJob> BatchListJobs(const std::vector<JobState>& states,
                                      const std::string& since);
  // Lists all files associated with a batch job.
  std::vector<BatchFileDesc> BatchListFiles(const std::string& job_id);
  // Returns the paths of the downloaded files.
  std::vector<std::filesystem::path> BatchDownload(
      const std::filesystem::path& output_dir, const std::string& job_id);
  // Returns the path of the downloaded file.
  std::filesystem::path BatchDownload(const std::filesystem::path& output_dir,
                                      const std::string& job_id,
                                      const std::string& filename_to_download);

  /*
   * Metadata API
   */

  // Retrievs a mapping of publisher name to publisher ID.
  std::vector<PublisherDetail> MetadataListPublishers();
  std::vector<std::string> MetadataListDatasets();
  std::vector<std::string> MetadataListDatasets(const DateRange& date_range);
  std::vector<Schema> MetadataListSchemas(const std::string& dataset);
  std::vector<FieldDetail> MetadataListFields(Encoding encoding, Schema schema);
  std::vector<UnitPricesForMode> MetadataListUnitPrices(const std::string& dataset);
  std::vector<DatasetConditionDetail> MetadataGetDatasetCondition(
      const std::string& dataset);
  std::vector<DatasetConditionDetail> MetadataGetDatasetCondition(
      const std::string& dataset, const DateRange& date_range);
  DatasetRange MetadataGetDatasetRange(const std::string& dataset);
  std::uint64_t MetadataGetRecordCount(const std::string& dataset,
                                       const DateTimeRange<UnixNanos>& datetime_range,
                                       const std::vector<std::string>& symbols,
                                       Schema schema);
  std::uint64_t MetadataGetRecordCount(const std::string& dataset,
                                       const DateTimeRange<std::string>& datetime_range,
                                       const std::vector<std::string>& symbols,
                                       Schema schema);
  std::uint64_t MetadataGetRecordCount(const std::string& dataset,
                                       const DateTimeRange<UnixNanos>& datetime_range,
                                       const std::vector<std::string>& symbols,
                                       Schema schema, SType stype_in,
                                       std::uint64_t limit);
  std::uint64_t MetadataGetRecordCount(const std::string& dataset,
                                       const DateTimeRange<std::string>& datetime_range,
                                       const std::vector<std::string>& symbols,
                                       Schema schema, SType stype_in,
                                       std::uint64_t limit);
  std::uint64_t MetadataGetBillableSize(const std::string& dataset,
                                        const DateTimeRange<UnixNanos>& datetime_range,
                                        const std::vector<std::string>& symbols,
                                        Schema schema);
  std::uint64_t MetadataGetBillableSize(
      const std::string& dataset, const DateTimeRange<std::string>& datetime_range,
      const std::vector<std::string>& symbols, Schema schema);
  std::uint64_t MetadataGetBillableSize(const std::string& dataset,
                                        const DateTimeRange<UnixNanos>& datetime_range,
                                        const std::vector<std::string>& symbols,
                                        Schema schema, SType stype_in,
                                        std::uint64_t limit);
  std::uint64_t MetadataGetBillableSize(
      const std::string& dataset, const DateTimeRange<std::string>& datetime_range,
      const std::vector<std::string>& symbols, Schema schema, SType stype_in,
      std::uint64_t limit);
  double MetadataGetCost(const std::string& dataset,
                         const DateTimeRange<UnixNanos>& datetime_range,
                         const std::vector<std::string>& symbols, Schema schema);
  double MetadataGetCost(const std::string& dataset,
                         const DateTimeRange<std::string>& datetime_range,
                         const std::vector<std::string>& symbols, Schema schema);
  double MetadataGetCost(const std::string& dataset,
                         const DateTimeRange<UnixNanos>& datetime_range,
                         const std::vector<std::string>& symbols, Schema schema,
                         FeedMode mode, SType stype_in, std::uint64_t limit);
  double MetadataGetCost(const std::string& dataset,
                         const DateTimeRange<std::string>& datetime_range,
                         const std::vector<std::string>& symbols, Schema schema,
                         FeedMode mode, SType stype_in, std::uint64_t limit);

  /*
   * Symbology API
   */

  SymbologyResolution SymbologyResolve(const std::string& dataset,
                                       const std::vector<std::string>& symbols,
                                       SType stype_in, SType stype_out,
                                       const DateRange& date_range);

  /*
   * Timeseries API
   */

  // Stream historical market data to `record_callback`. `metadata_callback`
  // will be called exactly once, before any calls to `record_callback`.
  // This method will return only after all data has been returned or
  // `record_callback` returns `KeepGoing::Stop`.
  //
  // NOTE: This method spawns a thread, however, the callbacks will be called
  // from the current thread.
  //
  // WARNING: Calling this method will incur a cost.
  void TimeseriesGetRange(const std::string& dataset,
                          const DateTimeRange<UnixNanos>& datetime_range,
                          const std::vector<std::string>& symbols, Schema schema,
                          const RecordCallback& record_callback);
  void TimeseriesGetRange(const std::string& dataset,
                          const DateTimeRange<std::string>& datetime_range,
                          const std::vector<std::string>& symbols, Schema schema,
                          const RecordCallback& record_callback);
  void TimeseriesGetRange(const std::string& dataset,
                          const DateTimeRange<UnixNanos>& datetime_range,
                          const std::vector<std::string>& symbols, Schema schema,
                          SType stype_in, SType stype_out, std::uint64_t limit,
                          const MetadataCallback& metadata_callback,
                          const RecordCallback& record_callback);
  void TimeseriesGetRange(const std::string& dataset,
                          const DateTimeRange<std::string>& datetime_range,
                          const std::vector<std::string>& symbols, Schema schema,
                          SType stype_in, SType stype_out, std::uint64_t limit,
                          const MetadataCallback& metadata_callback,
                          const RecordCallback& record_callback);
  // Stream historical market data to a file at `path`. Returns a `DbnFileStore`
  // object for replaying the data in `file_path`.
  //
  // If a file at `file_path` already exists, it will be overwritten.
  //
  // WARNING: Calling this method will incur a cost.
  DbnFileStore TimeseriesGetRangeToFile(const std::string& dataset,
                                        const DateTimeRange<UnixNanos>& datetime_range,
                                        const std::vector<std::string>& symbols,
                                        Schema schema,
                                        const std::filesystem::path& file_path);
  DbnFileStore TimeseriesGetRangeToFile(
      const std::string& dataset, const DateTimeRange<std::string>& datetime_range,
      const std::vector<std::string>& symbols, Schema schema,
      const std::filesystem::path& file_path);
  DbnFileStore TimeseriesGetRangeToFile(const std::string& dataset,
                                        const DateTimeRange<UnixNanos>& datetime_range,
                                        const std::vector<std::string>& symbols,
                                        Schema schema, SType stype_in, SType stype_out,
                                        std::uint64_t limit,
                                        const std::filesystem::path& file_path);
  DbnFileStore TimeseriesGetRangeToFile(
      const std::string& dataset, const DateTimeRange<std::string>& datetime_range,
      const std::vector<std::string>& symbols, Schema schema, SType stype_in,
      SType stype_out, std::uint64_t limit, const std::filesystem::path& file_path);

 private:
  friend HistoricalBuilder;

  using HttplibParams = std::multimap<std::string, std::string>;

  Historical(ILogReceiver* log_receiver, std::string key, HistoricalGateway gateway,
             VersionUpgradePolicy upgrade_policy, std::string user_agent_ext);
  Historical(ILogReceiver* log_receiver, std::string key, std::string gateway,
             std::uint16_t port, VersionUpgradePolicy upgrade_policy,
             std::string user_agent_ext);

  BatchJob BatchSubmitJob(const HttplibParams& params);
  void DownloadFile(const std::string& url, const std::filesystem::path& output_path);
  std::vector<BatchJob> BatchListJobs(const HttplibParams& params);
  std::vector<DatasetConditionDetail> MetadataGetDatasetCondition(
      const HttplibParams& params);
  std::uint64_t MetadataGetRecordCount(const HttplibParams& params);
  std::uint64_t MetadataGetBillableSize(const HttplibParams& params);
  double MetadataGetCost(const HttplibParams& params);
  void TimeseriesGetRange(const HttplibParams& params,
                          const MetadataCallback& metadata_callback,
                          const RecordCallback& record_callback);
  DbnFileStore TimeseriesGetRangeToFile(const HttplibParams& params,
                                        const std::filesystem::path& file_path);

  ILogReceiver* log_receiver_;
  const std::string key_;
  const std::string gateway_;
  const std::string user_agent_ext_;
  const VersionUpgradePolicy upgrade_policy_;
  detail::HttpClient client_;
};

// A helper class for constructing an instance of Historical.
class HistoricalBuilder {
 public:
  HistoricalBuilder() = default;

  /*
   * Required setters
   */

  // Sets `key_` based on the environment variable DATABENTO_API_KEY.
  //
  // NOTE: This is not thread-safe if `std::setenv` is used elsewhere in the
  // program.
  HistoricalBuilder& SetKeyFromEnv();
  HistoricalBuilder& SetKey(std::string key);

  /*
   * Optional setters
   */

  // Set the version upgrade policy for when streaming DBN data from a prior
  // version. Defaults to upgrading to DBNv3 (if not already).
  HistoricalBuilder& SetUpgradePolicy(VersionUpgradePolicy upgrade_policy);
  // Sets the receiver of the logs to be used by the client.
  HistoricalBuilder& SetLogReceiver(ILogReceiver* log_receiver);
  HistoricalBuilder& SetGateway(HistoricalGateway gateway);
  // Overrides the gateway and port. This is an advanced method.
  HistoricalBuilder& SetAddress(std::string gateway, std::uint16_t port);
  // Appends to the default user agent.
  HistoricalBuilder& ExtendUserAgent(std::string extension);

  // Attempts to construct an instance of Historical or throws an exception if
  // no key has been set.
  Historical Build();

 private:
  ILogReceiver* log_receiver_{};
  HistoricalGateway gateway_{HistoricalGateway::Bo1};
  std::string gateway_override_{};
  std::uint16_t port_{};
  std::string key_;
  VersionUpgradePolicy upgrade_policy_{VersionUpgradePolicy::UpgradeToV3};
  std::string user_agent_ext_;
};
}  // namespace databento
