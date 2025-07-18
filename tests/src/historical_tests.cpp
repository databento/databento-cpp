#include <date/date.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <httplib.h>
#include <nlohmann/json_fwd.hpp>

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <optional>
#include <stdexcept>  // logic_error
#include <utility>    // move

#include "databento/constants.hpp"
#include "databento/datetime.hpp"
#include "databento/dbn.hpp"
#include "databento/dbn_file_store.hpp"
#include "databento/enums.hpp"
#include "databento/exceptions.hpp"  // Exception
#include "databento/historical.hpp"
#include "databento/log.hpp"
#include "databento/metadata.hpp"
#include "databento/record.hpp"
#include "databento/symbology.hpp"  // kAllSymbols
#include "databento/timeseries.hpp"
#include "mock/mock_http_server.hpp"
#include "mock/mock_log_receiver.hpp"
#include "temp_file.hpp"

#ifdef _WIN32
namespace {
int setenv(const char* name, const char* value, int overwrite) {
  EXPECT_EQ(overwrite, 1) << "This shim only supports overwriting";
  return _putenv_s(name, value);
}

int unsetenv(const char* name) { return ::_putenv_s(name, ""); }
}  // namespace
#endif

namespace databento::tests {
constexpr auto kApiKey = "HIST_SECRET";

class HistoricalTests : public ::testing::Test {
 protected:
  Historical Client(int port) {
    return databento::HistoricalBuilder{}
        .SetLogReceiver(&logger_)
        .SetKey(kApiKey)
        .SetAddress("localhost", static_cast<std::uint16_t>(port))
        .Build();
  }

  std::filesystem::path tmp_path_{std::filesystem::temp_directory_path()};
  mock::MockHttpServer mock_server_{kApiKey};
  mock::MockLogReceiver logger_ =
      mock::MockLogReceiver::AssertNoLogs(LogLevel::Warning);
};

TEST_F(HistoricalTests, TestBatchSubmitJob) {
  const nlohmann::json kResp{
      {"actual_size", 2022690},
      {"billed_size", 5156064},
      {"compression", nullptr},
      {"cost_usd", 0.119089},
      {"dataset", "XNAS.ITCH"},
      {"delivery", "download"},
      {"encoding", "dbn"},
      {"end", "2022-07-03 00:00:00+00:00"},
      {"id", "GLBX-20221031-L3RVE95CV5"},
      {"limit", nullptr},
      {"package_size", 2026761},
      {"packaging", nullptr},
      {"pretty_px", false},
      {"pretty_ts", false},
      {"map_symbols", false},
      {"progress", 100},
      {"record_count", 107418},
      {"schema", "trades"},
      {"split_duration", "day"},
      {"split_size", nullptr},
      {"split_symbols", false},
      {"start", "2022-05-17 00:00:00+00:00"},
      {"state", "done"},
      {"stype_in", "raw_symbol"},
      {"stype_out", "instrument_id"},
      /* test the fact the API returns a string when there's only one symbol
       */
      {"symbols", "CLH3"},
      {"ts_expiration", "2022-11-30 15:29:43.148303+00:00"},
      {"ts_process_done", "2022-10-31 15:29:43.148303+00:00"},
      {"ts_process_start", "2022-10-31 15:29:41.189821+00:00"},
      {"ts_queued", "2022-10-31 15:29:39.130441+00:00"},
      {"ts_received", "2022-10-31 15:29:38.380286+00:00"},
      {"user_id", "TEST_USER"}};
  mock_server_.MockPostJson("/v0/batch.submit_job",
                            {{"dataset", dataset::kXnasItch},
                             {"start", "2022-05-17"},
                             {"end", "2022-07-03"},
                             {"encoding", "dbn"},
                             {"compression", "zstd"},
                             {"pretty_px", "0"},
                             {"pretty_ts", "0"},
                             {"map_symbols", "0"},
                             {"split_symbols", "0"},
                             {"symbols", "CLH3"},
                             {"schema", "trades"}},
                            kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target = Client(port);
  const auto res = target.BatchSubmitJob(dataset::kXnasItch, {"CLH3"}, Schema::Trades,
                                         {"2022-05-17", "2022-07-03"});
  EXPECT_EQ(res.symbols, std::vector<std::string>{"CLH3"});
  EXPECT_NEAR(res.cost_usd, 0.11908, 1e-2);
  EXPECT_EQ(res.encoding, Encoding::Dbn);
  // null handling
  EXPECT_EQ(res.compression, Compression::None);
  EXPECT_EQ(res.split_size, 0);
}

TEST_F(HistoricalTests, TestBatchListJobs) {
  const nlohmann::json kResp{{{"actual_size", 2022690},
                              {"billed_size", 5156064},
                              {"compression", "zstd"},
                              {"cost_usd", 0.119089},
                              {"dataset", "GLBX.MDP3"},
                              {"delivery", "download"},
                              {"encoding", "dbn"},
                              {"end", "2022-09-27 00:00:00+00:00"},
                              {"id", "CKXF"},
                              {"limit", nullptr},
                              {"package_size", 2026761},
                              {"packaging", nullptr},
                              {"pretty_px", false},
                              {"pretty_ts", false},
                              {"map_symbols", false},
                              {"progress", 100},
                              {"record_count", 107418},
                              {"schema", "trades"},
                              {"split_duration", "day"},
                              {"split_size", nullptr},
                              {"split_symbols", false},
                              {"start", "2022-08-26 00:00:00+00:00"},
                              {"state", "done"},
                              {"stype_in", "raw_symbol"},
                              {"stype_out", "instrument_id"},
                              {"symbols", "GEZ2"},
                              {"ts_expiration", "2022-11-30 15:27:10.148788+00:00"},
                              {"ts_process_done", "2022-10-31 15:27:10.148788+00:00"},
                              {"ts_process_start", "2022-10-31 15:27:08.018759+00:00"},
                              {"ts_queued", "2022-10-31 15:26:58.654241+00:00"},
                              {"ts_received", "2022-10-31 15:26:58.112496+00:00"},
                              {"user_id", "A_USER"}},
                             {{"actual_size", 2022690},
                              {"billed_size", 5156064},
                              {"compression", "zstd"},
                              {"cost_usd", 0.119089},
                              {"dataset", "GLBX.MDP3"},
                              {"delivery", "download"},
                              {"encoding", "dbn"},
                              {"end", "2022-09-27 00:00:00+00:00"},
                              {"id", "8UPL"},
                              {"limit", nullptr},
                              {"package_size", 2026761},
                              {"packaging", nullptr},
                              {"pretty_px", false},
                              {"pretty_ts", false},
                              {"map_symbols", false},
                              {"progress", 100},
                              {"record_count", 107418},
                              {"schema", "trades"},
                              {"split_duration", "day"},
                              {"split_size", nullptr},
                              {"split_symbols", false},
                              {"start", "2022-08-26 00:00:00+00:00"},
                              {"state", "done"},
                              {"stype_in", "raw_symbol"},
                              {"stype_out", "instrument_id"},
                              {"symbols", {"GEZ2", "GEH3"}},
                              {"ts_expiration", "2022-11-30 15:29:03.010429+00:00"},
                              {"ts_process_done", "2022-10-31 15:29:03.010429+00:00"},
                              {"ts_process_start", "2022-10-31 15:29:01.104930+00:00"},
                              {"ts_queued", "2022-10-31 15:28:58.933725+00:00"},
                              {"ts_received", "2022-10-31 15:28:58.233520+00:00"},
                              {"user_id", "A_USER"}}};
  mock_server_.MockGetJson("/v0/batch.list_jobs", kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target = Client(port);
  const auto res = target.BatchListJobs();
  ASSERT_EQ(res.size(), 2);
  const std::vector<std::string> symbols{"GEZ2", "GEH3"};
  EXPECT_EQ(res[1].symbols, symbols);
  EXPECT_EQ(res[0].ts_expiration, "2022-11-30 15:27:10.148788+00:00");
}

TEST_F(HistoricalTests, TestBatchListFiles) {
  const auto kJobId = "job123";
  const nlohmann::json kResp{
      {{"filename", "test.json"},
       {"size", 2148},
       {"hash", "9e7fe0b36"},
       {"urls",
        {{"https", "https://api.databento.com/v0/job_id/test.json"},
         {"ftp", "ftp://ftp.databento.com/job_id/test.json"}}}}};
  mock_server_.MockGetJson("/v0/batch.list_files", {{"job_id", kJobId}}, kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target = Client(port);
  const auto res = target.BatchListFiles(kJobId);
  ASSERT_EQ(res.size(), 1);
  const auto& file_desc = res[0];
  ASSERT_EQ(file_desc.filename, "test.json");
  ASSERT_EQ(file_desc.size, 2148);
  ASSERT_EQ(file_desc.hash, "9e7fe0b36");
  ASSERT_EQ(file_desc.https_url, "https://api.databento.com/v0/job_id/test.json");
  ASSERT_EQ(file_desc.ftp_url, "ftp://ftp.databento.com/job_id/test.json");
}

static const nlohmann::json kListFilesResp{
    {{"filename", "test.dbn"},
     {"size", {}},
     {"hash", {}},
     {"urls",
      {{"https", "https://api.databento.com/v0/job_id/test.dbn"},
       {"ftp", "ftp://fpt.databento.com/job_id/test.dbn"}}}},
    {{"filename", "test_metadata.json"},
     {"size", {}},
     {"hash", {}},
     {"urls",
      {{"https", "https://api.databento.com/v0/job_id/test_metadata.json"},
       {"ftp", "ftp://ftp.databento.com/job_id/test_metadata.json"}}}}};

TEST_F(HistoricalTests, TestBatchDownloadAll) {
  const auto kJobId = "job123";
  const TempFile temp_metadata_file{tmp_path_ / "job123/test_metadata.json"};
  const TempFile temp_dbn_file{tmp_path_ / "job123/test.dbn"};
  mock_server_.MockGetJson("/v0/batch.list_files", {{"job_id", kJobId}},
                           kListFilesResp);
  mock_server_.MockGetDbn("/v0/job_id/test.dbn", {},
                          TEST_DATA_DIR "/test_data.mbo.v3.dbn");
  mock_server_.MockGetJson("/v0/job_id/test_metadata.json", {{"key", "value"}});
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target = Client(port);
  ASSERT_FALSE(temp_metadata_file.Exists());
  ASSERT_FALSE(temp_dbn_file.Exists());
  const std::vector<std::filesystem::path> paths =
      target.BatchDownload(tmp_path_, kJobId);
  EXPECT_TRUE(temp_metadata_file.Exists());
  EXPECT_TRUE(temp_dbn_file.Exists());
  ASSERT_EQ(paths.size(), 2);
  EXPECT_NE(std::find_if(paths.begin(), paths.end(),
                         [&temp_metadata_file](const auto& path) {
                           return path.lexically_normal() ==
                                  temp_metadata_file.Path().lexically_normal();
                         }),
            paths.end());
  EXPECT_NE(std::find_if(
                paths.begin(), paths.end(),
                [&temp_dbn_file](const auto& path) {
                  return std::filesystem::path{path}.lexically_normal() ==
                         std::filesystem::path{temp_dbn_file.Path()}.lexically_normal();
                }),
            paths.end());
}

TEST_F(HistoricalTests, TestBatchDownloadSingle) {
  const auto kJobId = "654";
  const TempFile temp_metadata_file{tmp_path_ / "654/test_metadata.json"};
  mock_server_.MockGetJson("/v0/batch.list_files", {{"job_id", kJobId}},
                           kListFilesResp);
  mock_server_.MockGetJson("/v0/job_id/test_metadata.json", {{"key", "value"}});
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target = Client(port);
  ASSERT_FALSE(temp_metadata_file.Exists());
  const std::filesystem::path path =
      target.BatchDownload(tmp_path_, kJobId, "test_metadata.json");
  EXPECT_TRUE(temp_metadata_file.Exists());
  EXPECT_EQ(path.lexically_normal(), temp_metadata_file.Path().lexically_normal());
}

TEST_F(HistoricalTests, TestBatchDownloadSingleInvalidFile) {
  const auto kJobId = "654";
  mock_server_.MockGetJson("/v0/batch.list_files", {{"job_id", kJobId}},
                           kListFilesResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target = Client(port);
  ASSERT_THROW(target.BatchDownload(tmp_path_, kJobId, "test_metadata.js"),
               InvalidArgumentError);
}

TEST_F(HistoricalTests, TestMetadataListPublishers) {
  const nlohmann::json kResp{
      {{"publisher_id", 1},
       {"dataset", "GLBX.MDP3"},
       {"venue", "GLBX"},
       {"description", "CME Globex MDP 3.0"}},
      {{"publisher_id", 2},
       {"dataset", "XNAS.ITCH"},
       {"venue", "XNAS"},
       {"description", "Nasdaq TotalView-ITCH"}},
  };
  mock_server_.MockGetJson("/v0/metadata.list_publishers", kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target = Client(port);
  const auto res = target.MetadataListPublishers();
  EXPECT_EQ(res.size(), kResp.size());
  const auto glbx_exp = PublisherDetail{1, "GLBX.MDP3", "GLBX", "CME Globex MDP 3.0"};
  const auto xnas_exp =
      PublisherDetail{2, "XNAS.ITCH", "XNAS", "Nasdaq TotalView-ITCH"};
  EXPECT_EQ(res[0], glbx_exp);
  EXPECT_EQ(res[1], xnas_exp);
}

TEST_F(HistoricalTests, TestMetadataListDatasets_Simple) {
  const nlohmann::json kResp{
      dataset::kGlbxMdp3,
      dataset::kXnasItch,
  };
  mock_server_.MockGetJson("/v0/metadata.list_datasets", kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target = Client(port);
  const auto res = target.MetadataListDatasets();
  EXPECT_EQ(res.size(), kResp.size());
  EXPECT_EQ(res[0], kResp[0]);
  EXPECT_EQ(res[1], kResp[1]);
}

TEST_F(HistoricalTests, TestMetadataListDatasets_Full) {
  const nlohmann::json kResp{dataset::kXnasItch};
  mock_server_.MockGetJson("/v0/metadata.list_datasets", {{"start_date", "2021-01-05"}},
                           kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target = Client(port);
  const auto res = target.MetadataListDatasets(DateRange{"2021-01-05"});
  EXPECT_EQ(res.size(), kResp.size());
  EXPECT_EQ(res[0], kResp[0]);
}

TEST_F(HistoricalTests, TestMetadataListSchemas_Simple) {
  const nlohmann::json kResp{"mbo",      "mbp-1",    "mbp-10",   "tbbo",    "trades",
                             "ohlcv-1s", "ohlcv-1m", "ohlcv-1h", "ohlcv-1d"};
  mock_server_.MockGetJson("/v0/metadata.list_schemas",
                           {{"dataset", dataset::kGlbxMdp3}}, kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target = Client(port);
  const auto res = target.MetadataListSchemas(dataset::kGlbxMdp3);
  const std::vector<Schema> kExp{Schema::Mbo,     Schema::Mbp1,    Schema::Mbp10,
                                 Schema::Tbbo,    Schema::Trades,  Schema::Ohlcv1S,
                                 Schema::Ohlcv1M, Schema::Ohlcv1H, Schema::Ohlcv1D};
  ASSERT_EQ(res.size(), kResp.size());
  ASSERT_EQ(res.size(), kExp.size());
  for (std::size_t i = 0; i < res.size(); ++i) {
    EXPECT_EQ(res[i], kExp[i]) << "Index " << i;
  }
}

TEST_F(HistoricalTests, TestMetadataListSchemas_Full) {
  const nlohmann::json kResp{"mbo", "mbp-1", "ohlcv-1m", "ohlcv-1h", "ohlcv-1d"};
  mock_server_.MockGetJson("/v0/metadata.list_schemas",
                           {{"dataset", dataset::kGlbxMdp3}}, kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target = Client(port);
  const auto res = target.MetadataListSchemas(dataset::kGlbxMdp3);
  const std::vector<Schema> kExp{Schema::Mbo, Schema::Mbp1, Schema::Ohlcv1M,
                                 Schema::Ohlcv1H, Schema::Ohlcv1D};
  ASSERT_EQ(res.size(), kResp.size());
  ASSERT_EQ(res.size(), kExp.size());
  for (std::size_t i = 0; i < res.size(); ++i) {
    EXPECT_EQ(res[i], kExp[i]) << "Index " << i;
  }
}

TEST_F(HistoricalTests, TestMetadataListFields) {
  const nlohmann::json kResp{{{"name", "length"}, {"type", "uint8_t"}},
                             {{"name", "rtype"}, {"type", "uint8_t"}},
                             {{"name", "dataset_id"}, {"type", "uint16_t"}}};
  mock_server_.MockGetJson("/v0/metadata.list_fields",
                           {{"encoding", "dbn"}, {"schema", "trades"}}, kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target = Client(port);
  const auto res = target.MetadataListFields(Encoding::Dbn, Schema::Trades);
  const std::vector<FieldDetail> kExp{
      {"length", "uint8_t"}, {"rtype", "uint8_t"}, {"dataset_id", "uint16_t"}};
  EXPECT_EQ(res, kExp);
}

TEST_F(HistoricalTests, TestMetadataGetDatasetCondition) {
  const nlohmann::json kResp{{{"date", "2022-11-07"},
                              {"condition", "available"},
                              {"last_modified_date", "2023-03-01"}},
                             {{"date", "2022-11-08"},
                              {"condition", "degraded"},
                              {"last_modified_date", "2023-03-01"}},
                             {{"date", "2022-11-09"},
                              {"condition", "pending"},
                              {"last_modified_date", "2023-03-01"}},
                             {{"date", "2022-11-10"},
                              {"condition", "missing"},
                              {"last_modified_date", nullptr}}};
  mock_server_.MockGetJson("/v0/metadata.get_dataset_condition",
                           {{"dataset", dataset::kXnasItch},
                            {"start_date", "2022-11-06"},
                            {"end_date", "2022-11-10"}},
                           kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target = Client(port);
  const auto res = target.MetadataGetDatasetCondition(dataset::kXnasItch,
                                                      {"2022-11-06", "2022-11-10"});
  const std::vector<DatasetConditionDetail> kExp{
      {"2022-11-07", DatasetCondition::Available, "2023-03-01"},
      {"2022-11-08", DatasetCondition::Degraded, "2023-03-01"},
      {"2022-11-09", DatasetCondition::Pending, "2023-03-01"},
      {"2022-11-10", DatasetCondition::Missing, std::nullopt},
  };
  EXPECT_EQ(res, kExp);
}

TEST_F(HistoricalTests, TestMetadataListUnitPrices) {
  const nlohmann::json kResp{
      {{"mode", "historical-streaming"},
       {"unit_prices", {{"mbo", 21.05}, {"mbp-1", 82.05}, {"status", 62.72}}}}};
  mock_server_.MockGetJson("/v0/metadata.list_unit_prices",
                           {{"dataset", dataset::kGlbxMdp3}}, kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target = Client(port);
  const auto res = target.MetadataListUnitPrices(dataset::kGlbxMdp3);
  const UnitPricesForMode kExp{
      FeedMode::HistoricalStreaming,
      {{Schema::Mbo, 21.05}, {Schema::Mbp1, 82.05}, {Schema::Status, 62.72}}};
  ASSERT_EQ(res.size(), 1);
  EXPECT_EQ(res[0], kExp);
}

TEST_F(HistoricalTests, TestMetadataGetDatasetRange) {
  const nlohmann::json kResp = {{"start", "2017-05-21T00:00:00.000000000Z"},
                                {"end", "2022-12-01T00:00:00.000000000Z"},
                                {"schema",
                                 {
                                     {"bbo-1m",
                                      {{"start", "2020-08-02T00:00:00.000000000Z"},
                                       {"end", "2023-03-23T00:00:00.000000000Z"}}},
                                     {"ohlcv-1s",
                                      {{"start", "2020-08-02T00:00:00.000000000Z"},
                                       {"end", "2023-03-23T00:00:00.000000000Z"}}},
                                     {"ohlcv-1m",
                                      {{"start", "2020-08-02T00:00:00.000000000Z"},
                                       {"end", "2023-03-23T00:00:00.000000000Z"}}},

                                 }}};
  mock_server_.MockGetJson("/v0/metadata.get_dataset_range",
                           {{"dataset", dataset::kXnasItch}}, kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target = Client(port);
  const auto res = target.MetadataGetDatasetRange(dataset::kXnasItch);
  EXPECT_EQ(res.start, "2017-05-21T00:00:00.000000000Z");
  EXPECT_EQ(res.end, "2022-12-01T00:00:00.000000000Z");
}

TEST_F(HistoricalTests, TestMetadataGetRecordCount) {
  const nlohmann::json kResp = 42;
  mock_server_.MockPostJson("/v0/metadata.get_record_count",
                            {{"dataset", dataset::kGlbxMdp3},
                             {"symbols", "ESZ3,ESH4"},
                             {"start", "2020-06-06T00:00"},
                             {"end", "2021-03-02T00:00"},
                             {"schema", "trades"}},
                            kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target = Client(port);
  const auto res = target.MetadataGetRecordCount(
      dataset::kGlbxMdp3, {"2020-06-06T00:00", "2021-03-02T00:00"}, {"ESZ3", "ESH4"},
      Schema::Trades);
  ASSERT_EQ(res, kResp);
}

TEST_F(HistoricalTests, TestMetadataGetBillableSize_Simple) {
  const nlohmann::json kResp = 44688;
  mock_server_.MockPostJson("/v0/metadata.get_billable_size",
                            {{"dataset", dataset::kGlbxMdp3},
                             {"start", "2020-06-06T00:00"},
                             {"symbols", "ALL_SYMBOLS"},
                             {"end", "2021-03-02T00:00"},
                             {"schema", "trades"}},
                            kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target = Client(port);
  const auto res = target.MetadataGetBillableSize(
      dataset::kGlbxMdp3, {"2020-06-06T00:00", "2021-03-02T00:00"}, kAllSymbols,
      Schema::Trades);
  ASSERT_EQ(res, kResp);
}

TEST_F(HistoricalTests, TestMetadataGetBillableSize_Full) {
  const nlohmann::json kResp = 55238;
  mock_server_.MockPostJson("/v0/metadata.get_billable_size",
                            {{"dataset", dataset::kGlbxMdp3},
                             {"start", "2020-06-06T00:00"},
                             {"end", "2021-03-02T00:00"},
                             {"symbols", "NG.FUT,LNG.FUT"},
                             {"schema", "tbbo"},
                             {"stype_in", "parent"}},
                            kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target = Client(port);
  const auto res = target.MetadataGetBillableSize(
      dataset::kGlbxMdp3, {"2020-06-06T00:00", "2021-03-02T00:00"},
      {"NG.FUT", "LNG.FUT"}, Schema::Tbbo, SType::Parent, {});
  ASSERT_EQ(res, kResp);
}

TEST_F(HistoricalTests, TestMetadataGetCost_Simple) {
  const nlohmann::json kResp = 0.65783;
  mock_server_.MockPostJson("/v0/metadata.get_cost",
                            {{"dataset", dataset::kGlbxMdp3},
                             {"start", "2020-06-06T00:00"},
                             {"end", "2021-03-02T00:00"},
                             {"symbols", "MESN1,MESQ1"},
                             {"schema", "trades"}},
                            kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target = Client(port);
  const auto res = target.MetadataGetCost(dataset::kGlbxMdp3,
                                          {"2020-06-06T00:00", "2021-03-02T00:00"},
                                          {"MESN1", "MESQ1"}, Schema::Trades);
  ASSERT_DOUBLE_EQ(res, kResp);
}

TEST_F(HistoricalTests, TestMetadataGetCost_Full) {
  const nlohmann::json kResp = 0.714;
  mock_server_.MockPostJson("/v0/metadata.get_cost",
                            {{"dataset", dataset::kGlbxMdp3},
                             {"start", "2020-06-06T00:00"},
                             {"end", "2021-03-02T00:00"},
                             {"mode", "historical-streaming"},
                             {"symbols", "MES.OPT,EW.OPT"},
                             {"schema", "tbbo"},
                             {"stype_in", "parent"}},
                            kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target = Client(port);
  const auto res = target.MetadataGetCost(
      dataset::kGlbxMdp3, {"2020-06-06T00:00", "2021-03-02T00:00"},
      {"MES.OPT", "EW.OPT"}, Schema::Tbbo, FeedMode::HistoricalStreaming, SType::Parent,
      {});
  ASSERT_DOUBLE_EQ(res, kResp);
}

TEST_F(HistoricalTests, TestSymbologyResolve) {
  const nlohmann::json kResp{
      {"result",
       {{"ESM2",
         {{
             {"d0", "2022-06-06"},
             {"d1", "2022-06-10"},
             {"s", "3403"},
         }}}}},
      {"symbols", {"ESM2"}},
      {"stype_in", "raw_symbol"},
      {"stype_out", "instrument_id"},
      {"start_date", "2022-06-06"},
      {"end_date", "2022-06-10"},
      {"partial", nlohmann::json::array()},
      {"not_found", nlohmann::json::array()},
      {"message", "OK"},
      {"status", 0},
  };

  mock_server_.MockPostJson("/v0/symbology.resolve",
                            {
                                {"dataset", dataset::kGlbxMdp3},
                                {"start_date", "2022-06-06"},
                                {"end_date", "2022-06-10"},
                                {"symbols", "ESM2"},
                                {"stype_in", "raw_symbol"},
                                {"stype_out", "instrument_id"},
                            },
                            kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target = Client(port);
  const auto res =
      target.SymbologyResolve(dataset::kGlbxMdp3, {"ESM2"}, SType::RawSymbol,
                              SType::InstrumentId, {"2022-06-06", "2022-06-10"});
  EXPECT_TRUE(res.not_found.empty());
  EXPECT_TRUE(res.partial.empty());
  ASSERT_EQ(res.mappings.size(), 1);
  const auto& esm2_mappings = res.mappings.at("ESM2");
  ASSERT_EQ(esm2_mappings.size(), 1);
  const auto& esm2_mapping = esm2_mappings.at(0);
  EXPECT_EQ(esm2_mapping.start_date, date::year{2022} / 6 / 6);
  EXPECT_EQ(esm2_mapping.end_date, date::year{2022} / 6 / 10);
  EXPECT_EQ(esm2_mapping.symbol, "3403");
}

TEST_F(HistoricalTests, TestTimeseriesGetRange_Basic) {
  mock_server_.MockPostDbn("/v0/timeseries.get_range",
                           {{"dataset", dataset::kGlbxMdp3},
                            {"symbols", "ESH1"},
                            {"schema", "mbo"},
                            {"start", "1609160400000711344"},
                            {"end", "1609160800000711344"},
                            {"encoding", "dbn"},
                            {"stype_in", "raw_symbol"},
                            {"stype_out", "instrument_id"},
                            {"limit", "2"}},
                           TEST_DATA_DIR "/test_data.mbo.v3.dbn.zst");
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target = Client(port);
  std::unique_ptr<Metadata> metadata_ptr;
  std::vector<MboMsg> mbo_records;
  target.TimeseriesGetRange(
      dataset::kGlbxMdp3,
      {UnixNanos{std::chrono::nanoseconds{1609160400000711344}},
       UnixNanos{std::chrono::nanoseconds{1609160800000711344}}},
      {"ESH1"}, Schema::Mbo, SType::RawSymbol, SType::InstrumentId, 2,
      [&metadata_ptr](Metadata&& metadata) {
        metadata_ptr = std::make_unique<Metadata>(std::move(metadata));
      },
      [&mbo_records](const Record& record) {
        mbo_records.emplace_back(record.Get<MboMsg>());
        return KeepGoing::Continue;
      });
  ASSERT_NE(metadata_ptr, nullptr) << "metadata callback wasn't called";
  EXPECT_EQ(metadata_ptr->limit, 2);
  EXPECT_EQ(metadata_ptr->schema, Schema::Mbo);
  EXPECT_EQ(mbo_records.size(), 2);
}

TEST_F(HistoricalTests, TestTimeseriesGetRange_NoMetadataCallback) {
  mock_server_.MockPostDbn("/v0/timeseries.get_range",
                           {{"dataset", dataset::kGlbxMdp3},
                            {"start", "2022-10-21T13:30"},
                            {"end", "2022-10-21T20:00"},
                            {"symbols", "CYZ2"},
                            {"schema", "tbbo"},
                            {"encoding", "dbn"},
                            {"stype_in", "raw_symbol"},
                            {"stype_out", "instrument_id"}},
                           TEST_DATA_DIR "/test_data.tbbo.v3.dbn.zst");
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target = Client(port);
  std::vector<TbboMsg> mbo_records;
  target.TimeseriesGetRange(dataset::kGlbxMdp3,
                            {"2022-10-21T13:30", "2022-10-21T20:00"}, {"CYZ2"},
                            Schema::Tbbo, [&mbo_records](const Record& record) {
                              mbo_records.emplace_back(record.Get<TbboMsg>());
                              return KeepGoing::Continue;
                            });
  EXPECT_EQ(mbo_records.size(), 2);
}

// should get helpful message if there's a problem with the request
TEST_F(HistoricalTests, TestTimeseriesGetRange_BadRequest) {
  const nlohmann::json resp{
      {"detail", "Authorization failed: illegal chars in username."}};
  mock_server_.MockBadPostRequest("/v0/timeseries.get_range", resp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target = Client(port);
  try {
    target.TimeseriesGetRange(
        dataset::kGlbxMdp3,
        {UnixNanos{std::chrono::nanoseconds{1609160400000711344}},
         UnixNanos{std::chrono::nanoseconds{1609160800000711344}}},
        {"E5A.OPT"}, Schema::Mbo, SType::Parent, SType::InstrumentId, 2,
        [](Metadata&&) {}, [](const Record&) { return KeepGoing::Continue; });
    FAIL() << "Call to TimeseriesGetRange was supposed to throw";
  } catch (const std::exception& exc) {
    ASSERT_STREQ(exc.what(),
                 "Received an error response from request to "
                 "/v0/timeseries.get_range with status 400 and body "
                 "'{\"detail\":\"Authorization failed: illegal chars in username.\"}'"

    );
  }
}

TEST_F(HistoricalTests, TestTimeseriesGetRange_CallbackException) {
  mock_server_.MockPostDbn("/v0/timeseries.get_range", {},
                           TEST_DATA_DIR "/test_data.mbo.v3.dbn.zst");
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target = Client(port);
  ASSERT_THROW(target.TimeseriesGetRange(
                   dataset::kGlbxMdp3,
                   {UnixNanos{std::chrono::nanoseconds{1609160400000711344}},
                    UnixNanos{std::chrono::nanoseconds{1609160800000711344}}},
                   {"ESH1"}, Schema::Mbo, SType::RawSymbol, SType::InstrumentId, 2,
                   [](Metadata&&) { throw std::logic_error{"Test failure"}; },
                   [](const Record&) { return KeepGoing::Continue; }),
               std::logic_error);
}

TEST_F(HistoricalTests, TestTimeseriesGetRange_Cancellation) {
  mock_server_.MockPostDbn("/v0/timeseries.get_range", {},
                           TEST_DATA_DIR "/test_data.mbo.v3.dbn.zst");
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target = Client(port);
  std::uint32_t call_count = 0;
  target.TimeseriesGetRange(
      dataset::kGlbxMdp3,
      {UnixNanos{std::chrono::nanoseconds{1609160400000711344}},
       UnixNanos{std::chrono::nanoseconds{1609160800000711344}}},
      {"ESH1"}, Schema::Mbo, SType::RawSymbol, SType::InstrumentId, 2,
      [](Metadata&&) {},
      [&call_count](const Record&) {
        ++call_count;
        return KeepGoing::Stop;
      });
  // Should gracefully exit after first record, even though there are two
  // records in the file
  ASSERT_EQ(call_count, 1);
}

TEST_F(HistoricalTests, TestTimeseriesGetRange_LargeChunks) {
  Mbp1Msg mbp1{RecordHeader{sizeof(Mbp1Msg) / kRecordHeaderLengthMultiplier,
                            RType::Mbp1,
                            static_cast<std::uint16_t>(Publisher::IfusImpactIfus),
                            10005,
                            {}}};
  constexpr auto kRecordCount = 50'000;
  mock_server_.MockPostDbn("/v0/timeseries.get_range",
                           {{"dataset", ToString(Dataset::IfusImpact)}},
                           Record{&mbp1.hd}, kRecordCount, 75'000);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target = Client(port);
  std::size_t counter = 0;
  target.TimeseriesGetRange(ToString(Dataset::IfusImpact), {"2024-05", "2025-05"},
                            kAllSymbols, Schema::Mbp1,
                            [&counter, &mbp1](const Record& record) {
                              ++counter;
                              EXPECT_TRUE(record.Holds<Mbp1Msg>());
                              EXPECT_EQ(record.Get<Mbp1Msg>(), mbp1);
                              return KeepGoing::Continue;
                            });
  EXPECT_EQ(counter, kRecordCount);
}

TEST_F(HistoricalTests, TestTimeseriesGetRange_UnreadBytes) {
  Mbp1Msg mbp1{RecordHeader{sizeof(Mbp1Msg) / kRecordHeaderLengthMultiplier,
                            RType::Mbp1,
                            static_cast<std::uint16_t>(Publisher::IfusImpactIfus),
                            10005,
                            {}}};
  constexpr auto kRecordCount = 1'000;
  mock_server_.MockPostDbn("/v0/timeseries.get_range",
                           {{"dataset", ToString(Dataset::IfusImpact)}},
                           Record{&mbp1.hd}, kRecordCount, 20, 75'000);
  const auto port = mock_server_.ListenOnThread();

  logger_ =
      mock::MockLogReceiver{[](auto count, LogLevel level, const std::string& msg) {
        EXPECT_THAT(msg, testing::EndsWith(
                             "Partial or incomplete record remaining of 20 bytes"));
      }};

  databento::Historical target = Client(port);
  std::size_t counter = 0;
  target.TimeseriesGetRange(ToString(Dataset::IfusImpact), {"2024-05", "2025-05"},
                            kAllSymbols, Schema::Mbp1,
                            [&counter, &mbp1](const Record& record) {
                              ++counter;
                              EXPECT_TRUE(record.Holds<Mbp1Msg>());
                              EXPECT_EQ(record.Get<Mbp1Msg>(), mbp1);
                              return KeepGoing::Continue;
                            });
  EXPECT_EQ(counter, kRecordCount);
  ASSERT_EQ(logger_.CallCount(), 1);
}

TEST_F(HistoricalTests, TestTimeseriesGetRangeToFile) {
  mock_server_.MockPostDbn("/v0/timeseries.get_range",
                           {{"dataset", dataset::kGlbxMdp3},
                            {"start", "2022-10-21T13:30"},
                            {"end", "2022-10-21T20:00"},
                            {"symbols", "CYZ2"},
                            {"schema", "tbbo"},
                            {"encoding", "dbn"},
                            {"stype_in", "raw_symbol"},
                            {"stype_out", "instrument_id"}},
                           TEST_DATA_DIR "/test_data.tbbo.v3.dbn.zst");
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target = Client(port);
  const TempFile temp_file{testing::TempDir() + "/TestTimeseriesGetRangeToFile"};
  target.TimeseriesGetRangeToFile(dataset::kGlbxMdp3,
                                  {"2022-10-21T13:30", "2022-10-21T20:00"}, {"CYZ2"},
                                  Schema::Tbbo, temp_file.Path());
  // running it a second time should overwrite previous data
  DbnFileStore bento = target.TimeseriesGetRangeToFile(
      dataset::kGlbxMdp3, {"2022-10-21T13:30", "2022-10-21T20:00"}, {"CYZ2"},
      Schema::Tbbo, temp_file.Path());
  std::size_t counter{};
  bento.Replay([&counter](const Record&) {
    ++counter;
    return KeepGoing::Continue;
  });
  ASSERT_EQ(counter, 2);
}

TEST(JsonImplementationTests, TestParsingNumberNotPreciselyRepresentableAsDouble) {
  auto const number_json = nlohmann::json::parse("1609160400000711344");
  EXPECT_TRUE(number_json.is_number());
  EXPECT_EQ(number_json, 1609160400000711344);
}

TEST(HistoricalBuilderTests, TestBasic) {
  constexpr auto kKey = "SECRET";

  const auto client = databento::HistoricalBuilder()
                          .SetKey(kKey)
                          .SetGateway(databento::HistoricalGateway::Bo1)
                          .Build();
  EXPECT_EQ(client.Key(), kKey);
  EXPECT_EQ(client.Gateway(), "https://hist.databento.com");
}

TEST(HistoricalBuilderTests, TestMissingKey) {
  ASSERT_THROW(databento::HistoricalBuilder().Build(), Exception);
}

TEST(HistoricalBuilderTests, TestSetKeyFromEnv) {
  constexpr auto kKey = "SECRET_KEY";
  ASSERT_EQ(::setenv("DATABENTO_API_KEY", kKey, 1), 0)
      << "Failed to set environment variable";
  const auto client = databento::HistoricalBuilder().SetKeyFromEnv().Build();
  EXPECT_EQ(client.Key(), kKey);
  EXPECT_EQ(client.Gateway(), "https://hist.databento.com");
  // unsetting prevents this test from affecting others
  ASSERT_EQ(::unsetenv("DATABENTO_API_KEY"), 0)
      << "Failed to unset environment variable";
}

TEST(HistoricalBuilderTests, TestSetKeyFromEnvMissing) {
  ASSERT_EQ(::unsetenv("DATABENTO_API_KEY"), 0) << "Failed to set environment variable";
  ASSERT_THROW(databento::HistoricalBuilder().SetKeyFromEnv().Build(), Exception);
}
}  // namespace databento::tests
