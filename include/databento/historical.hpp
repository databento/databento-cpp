#pragma once

#include <cstddef>  // size_t
#include <cstdint>
#include <map>  // map, multimap
#include <string>
#include <vector>

#include "databento/batch.hpp"               // BatchJob
#include "databento/datetime.hpp"            // UnixNanos
#include "databento/detail/http_client.hpp"  // HttpClient
#include "databento/enums.hpp"  // BatchState, Delivery, DurationInterval, Packaging, Schema, SType
#include "databento/file_bento.hpp"
#include "databento/metadata.hpp"  // DatasetConditionInfo, FieldsByDatasetEncodingAndSchema, PriceByFeedMode, PriceByFeedModeAndSchema, PriceBySchema
#include "databento/symbology.hpp"  // SymbologyResolution
#include "databento/timeseries.hpp"  // KeepGoing, MetadataCallback, RecordCallback

namespace databento {
// A client for interfacing with Databento's historical market data API.
class Historical {
 public:
  Historical(std::string key, HistoricalGateway gateway);
  // Primarily for unit tests
  Historical(std::string key, std::string gateway, std::uint16_t port);

  /*
   * Getters
   */

  const std::string& Key() const { return key_; };
  const std::string& Gateway() const { return gateway_; }

  /*
   * Batch API
   */

  BatchJob BatchSubmitJob(const std::string& dataset, UnixNanos start,
                          UnixNanos end,
                          const std::vector<std::string>& symbols,
                          Schema schema);
  BatchJob BatchSubmitJob(const std::string& dataset, const std::string& start,
                          const std::string& end,
                          const std::vector<std::string>& symbols,
                          Schema schema);
  BatchJob BatchSubmitJob(const std::string& dataset, UnixNanos start,
                          UnixNanos end,
                          const std::vector<std::string>& symbols,
                          Schema schema, SplitDuration split_duration,
                          std::size_t split_size, Packaging packaging,
                          Delivery delivery, SType stype_in, SType stype_out,
                          std::size_t limit);
  BatchJob BatchSubmitJob(const std::string& dataset, const std::string& start,
                          const std::string& end,
                          const std::vector<std::string>& symbols,
                          Schema schema, SplitDuration split_duration,
                          std::size_t split_size, Packaging packaging,
                          Delivery delivery, SType stype_in, SType stype_out,
                          std::size_t limit);
  std::vector<BatchJob> BatchListJobs();
  std::vector<BatchJob> BatchListJobs(const std::vector<JobState>& states,
                                      UnixNanos since);
  std::vector<BatchJob> BatchListJobs(const std::vector<JobState>& states,
                                      const std::string& since);

  /*
   * Metadata API
   */

  // Retrievs a mapping of publisher name to publisher ID.
  std::map<std::string, std::int32_t> MetadataListPublishers();
  std::vector<std::string> MetadataListDatasets();
  std::vector<std::string> MetadataListDatasets(const std::string& start_date,
                                                const std::string& end_date);
  std::vector<Schema> MetadataListSchemas(const std::string& dataset);
  std::vector<Schema> MetadataListSchemas(const std::string& dataset,
                                          const std::string& start_date,
                                          const std::string& end_date);
  FieldsByDatasetEncodingAndSchema MetadataListFields();
  FieldsByDatasetEncodingAndSchema MetadataListFields(
      const std::string& dataset);
  FieldsByDatasetEncodingAndSchema MetadataListFields(
      const std::string& dataset, Encoding encoding, Schema schema);
  std::vector<Encoding> MetadataListEncodings();
  std::vector<Compression> MetadataListCompressions();
  PriceByFeedModeAndSchema MetadataListUnitPrices(const std::string& dataset);
  PriceBySchema MetadataListUnitPrices(const std::string& dataset,
                                       FeedMode mode);
  PriceByFeedMode MetadataListUnitPrices(const std::string& dataset,
                                         Schema schema);
  double MetadataListUnitPrices(const std::string& dataset, FeedMode mode,
                                Schema schema);
  DatasetConditionInfo MetadataGetDatasetCondition(
      const std::string& dataset, const std::string& start_date,
      const std::string& end_date);
  std::size_t MetadataGetRecordCount(const std::string& dataset,
                                     UnixNanos start, UnixNanos end,
                                     const std::vector<std::string>& symbols,
                                     Schema schema);
  std::size_t MetadataGetRecordCount(const std::string& dataset,
                                     const std::string& start,
                                     const std::string& end,
                                     const std::vector<std::string>& symbols,
                                     Schema schema);
  std::size_t MetadataGetRecordCount(const std::string& dataset,
                                     UnixNanos start, UnixNanos end,
                                     const std::vector<std::string>& symbols,
                                     Schema schema, SType stype_in,
                                     std::size_t limit);
  std::size_t MetadataGetRecordCount(const std::string& dataset,
                                     const std::string& start,
                                     const std::string& end,
                                     const std::vector<std::string>& symbols,
                                     Schema schema, SType stype_in,
                                     std::size_t limit);
  std::size_t MetadataGetBillableSize(const std::string& dataset,
                                      UnixNanos start, UnixNanos end,
                                      const std::vector<std::string>& symbols,
                                      Schema schema);
  std::size_t MetadataGetBillableSize(const std::string& dataset,
                                      const std::string& start,
                                      const std::string& end,
                                      const std::vector<std::string>& symbols,
                                      Schema schema);
  std::size_t MetadataGetBillableSize(const std::string& dataset,
                                      UnixNanos start, UnixNanos end,
                                      const std::vector<std::string>& symbols,
                                      Schema schema, SType stype_in,
                                      std::size_t limit);
  std::size_t MetadataGetBillableSize(const std::string& dataset,
                                      const std::string& start,
                                      const std::string& end,
                                      const std::vector<std::string>& symbols,
                                      Schema schema, SType stype_in,
                                      std::size_t limit);
  double MetadataGetCost(const std::string& dataset, UnixNanos start,
                         UnixNanos end, const std::vector<std::string>& symbols,
                         Schema schema);
  double MetadataGetCost(const std::string& dataset, const std::string& start,
                         const std::string& end,
                         const std::vector<std::string>& symbols,
                         Schema schema);
  double MetadataGetCost(const std::string& dataset, UnixNanos start,
                         UnixNanos end, const std::vector<std::string>& symbols,
                         Schema schema, FeedMode mode, SType stype_in,
                         std::size_t limit);
  double MetadataGetCost(const std::string& dataset, const std::string& start,
                         const std::string& end,
                         const std::vector<std::string>& symbols, Schema schema,
                         FeedMode mode, SType stype_in, std::size_t limit);

  /*
   * Symbology API
   */

  SymbologyResolution SymbologyResolve(const std::string& dataset,
                                       const std::string& start_date,
                                       const std::string& end_date,
                                       const std::vector<std::string>& symbols,
                                       SType stype_in, SType stype_out);
  SymbologyResolution SymbologyResolve(const std::string& dataset,
                                       const std::string& start_date,
                                       const std::string& end_date,
                                       const std::vector<std::string>& symbols,
                                       SType stype_in, SType stype_out,
                                       const std::string& default_value);

  /*
   * Timeseries API
   */

  // Stream historical market data to `record_callback`. This method will
  // return only after all data has been returned or `record_callback` returns
  // `KeepGoing::Stop`.
  //
  // NOTE: This method spawns a thread, however, the callbacks will be called
  // from the current thread.
  void TimeseriesStream(const std::string& dataset, UnixNanos start,
                        UnixNanos end, const std::vector<std::string>& symbols,
                        Schema schema, const RecordCallback& record_callback);
  void TimeseriesStream(const std::string& dataset, const std::string& start,
                        const std::string& end,
                        const std::vector<std::string>& symbols, Schema schema,
                        const RecordCallback& record_callback);
  // Stream historical market data to `record_callback`. `metadata_callback`
  // will be called exactly once, before any calls to `record_callback`.
  // This method will return only after all data has been returned or
  // `record_callback` returns `KeepGoing::Stop`.
  //
  // NOTE: This method spawns a thread, however, the callbacks will be called
  // from the current thread.
  void TimeseriesStream(const std::string& dataset, UnixNanos start,
                        UnixNanos end, const std::vector<std::string>& symbols,
                        Schema schema, SType stype_in, SType stype_out,
                        std::size_t limit,
                        const MetadataCallback& metadata_callback,
                        const RecordCallback& record_callback);
  void TimeseriesStream(const std::string& dataset, const std::string& start,
                        const std::string& end,
                        const std::vector<std::string>& symbols, Schema schema,
                        SType stype_in, SType stype_out, std::size_t limit,
                        const MetadataCallback& metadata_callback,
                        const RecordCallback& record_callback);
  // Stream historical market data to a file at `path`. Returns a `FileBento`
  // object for replaying the data in `file_path`.
  //
  // If a file at `file_path` already exists, it will be overwritten.
  FileBento TimeseriesStreamToFile(const std::string& dataset, UnixNanos start,
                                   UnixNanos end,
                                   const std::vector<std::string>& symbols,
                                   Schema schema, const std::string& file_path);
  FileBento TimeseriesStreamToFile(const std::string& dataset,
                                   const std::string& start,
                                   const std::string& end,
                                   const std::vector<std::string>& symbols,
                                   Schema schema, const std::string& file_path);
  FileBento TimeseriesStreamToFile(const std::string& dataset, UnixNanos start,
                                   UnixNanos end,
                                   const std::vector<std::string>& symbols,
                                   Schema schema, SType stype_in,
                                   SType stype_out, std::size_t limit,
                                   const std::string& file_path);
  FileBento TimeseriesStreamToFile(const std::string& dataset,
                                   const std::string& start,
                                   const std::string& end,
                                   const std::vector<std::string>& symbols,
                                   Schema schema, SType stype_in,
                                   SType stype_out, std::size_t limit,
                                   const std::string& file_path);

 private:
  using HttplibParams = std::multimap<std::string, std::string>;

  BatchJob BatchSubmitJob(const HttplibParams& params);
  std::vector<BatchJob> BatchListJobs(const HttplibParams& params);
  std::size_t MetadataGetRecordCount(const HttplibParams& params);
  std::size_t MetadataGetBillableSize(const HttplibParams& params);
  double MetadataGetCost(const HttplibParams& params);
  FieldsByDatasetEncodingAndSchema MetadataListFields(
      const HttplibParams& params);
  void TimeseriesStream(const HttplibParams& params,
                        const MetadataCallback& metadata_callback,
                        const RecordCallback& record_callback);
  FileBento TimeseriesStreamToFile(const HttplibParams& params,
                                   const std::string& file_path);

  const std::string key_;
  const std::string gateway_;
  detail::HttpClient client_;
};

// A helper class for constructing an instance of Historical.
class HistoricalBuilder {
 public:
  HistoricalBuilder() = default;

  // Sets `key_` based on the environment variable DATABENTO_API_KEY.
  //
  // NOTE: This is not thread-safe if `std::setenv` is used elsewhere in the
  // program.
  HistoricalBuilder& SetKeyFromEnv();
  HistoricalBuilder& SetKey(std::string key);
  HistoricalBuilder& SetGateway(HistoricalGateway gateway);
  // Attempts to construct an instance of Historical or throws an exception if
  // no key has been set.
  Historical Build();

 private:
  std::string key_;
  HistoricalGateway gateway_{HistoricalGateway::Bo1};
};
}  // namespace databento
