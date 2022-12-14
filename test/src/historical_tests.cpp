#include <gtest/gtest.h>
#include <httplib.h>
#include <nlohmann/json_fwd.hpp>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <stdexcept>  // logic_error
#include <thread>

#include "databento/constants.hpp"
#include "databento/datetime.hpp"
#include "databento/dbz.hpp"
#include "databento/enums.hpp"
#include "databento/exceptions.hpp"  // Exception
#include "databento/file_bento.hpp"
#include "databento/historical.hpp"
#include "databento/metadata.hpp"
#include "databento/record.hpp"
#include "databento/symbology.hpp"  // kAllSymbols
#include "databento/timeseries.hpp"
#include "mock/mock_http_server.hpp"
#include "temp_file.hpp"

namespace databento {
namespace test {
constexpr auto kApiKey = "HIST_SECRET";

class HistoricalTests : public ::testing::Test {
 protected:
  mock::MockHttpServer mock_server_{kApiKey};
};

TEST_F(HistoricalTests, TestBatchSubmitJob) {
  const nlohmann::json kResp{
      {"actual_size", 2022690},
      {"bill_id", "73186317471eb623d161a1"},
      {"billed_size", 5156064},
      {"compression", "zstd"},
      {"cost", 11.9089},
      {"dataset", "XNAS.ITCH"},
      {"delivery", "download"},
      {"encoding", "dbz"},
      {"end", "2022-07-03 00:00:00+00:00"},
      {"id", "GLBX-20221031-L3RVE95CV5"},
      {"is_example", false},
      {"is_full_universe", false},
      {"limit", nullptr},
      {"package_size", 2026761},
      {"packaging", "none"},
      {"pretty_px", false},
      {"pretty_ts", false},
      {"progress", 100},
      {"record_count", 107418},
      {"schema", "trades"},
      {"split_duration", "day"},
      {"split_size", nullptr},
      {"split_symbols", false},
      {"start", "2022-05-17 00:00:00+00:00"},
      {"state", "done"},
      {"stype_in", "native"},
      {"stype_out", "product_id"},
      /* test the fact the API returns a string when there's only one symbol */
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
                             {"symbols", "CLH3"},
                             {"schema", "trades"}},
                            kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target{kApiKey, "localhost",
                               static_cast<std::uint16_t>(port)};
  const auto res = target.BatchSubmitJob(
      dataset::kXnasItch, "2022-05-17", "2022-07-03", {"CLH3"}, Schema::Trades);
  EXPECT_EQ(res.symbols, std::vector<std::string>{"CLH3"});
  EXPECT_NEAR(res.cost, 11.908, 1e-2);
  EXPECT_EQ(res.encoding, Encoding::Dbz);
  // null handling
  EXPECT_EQ(res.split_size, 0);
}

TEST_F(HistoricalTests, TestBatchListJobs) {
  const nlohmann::json kResp{
      {{"actual_size", 2022690},
       {"bill_id", "a670"},
       {"billed_size", 5156064},
       {"compression", "zstd"},
       {"cost", 11.9089},
       {"dataset", "GLBX.MDP3"},
       {"delivery", "download"},
       {"encoding", "dbz"},
       {"end", "2022-09-27 00:00:00+00:00"},
       {"id", "CKXF"},
       {"is_example", false},
       {"is_full_universe", false},
       {"limit", nullptr},
       {"package_size", 2026761},
       {"packaging", "none"},
       {"pretty_px", false},
       {"pretty_ts", false},
       {"progress", 100},
       {"record_count", 107418},
       {"schema", "trades"},
       {"split_duration", "day"},
       {"split_size", nullptr},
       {"split_symbols", false},
       {"start", "2022-08-26 00:00:00+00:00"},
       {"state", "done"},
       {"stype_in", "native"},
       {"stype_out", "product_id"},
       {"symbols", "GEZ2"},
       {"ts_expiration", "2022-11-30 15:27:10.148788+00:00"},
       {"ts_process_done", "2022-10-31 15:27:10.148788+00:00"},
       {"ts_process_start", "2022-10-31 15:27:08.018759+00:00"},
       {"ts_queued", "2022-10-31 15:26:58.654241+00:00"},
       {"ts_received", "2022-10-31 15:26:58.112496+00:00"},
       {"user_id", "A_USER"}},
      {{"actual_size", 2022690},
       {"bill_id", "a1b7"},
       {"billed_size", 5156064},
       {"compression", "zstd"},
       {"cost", 11.9089},
       {"dataset", "GLBX.MDP3"},
       {"delivery", "download"},
       {"encoding", "dbz"},
       {"end", "2022-09-27 00:00:00+00:00"},
       {"id", "8UPL"},
       {"is_example", false},
       {"is_full_universe", false},
       {"limit", nullptr},
       {"package_size", 2026761},
       {"packaging", "none"},
       {"pretty_px", false},
       {"pretty_ts", false},
       {"progress", 100},
       {"record_count", 107418},
       {"schema", "trades"},
       {"split_duration", "day"},
       {"split_size", nullptr},
       {"split_symbols", false},
       {"start", "2022-08-26 00:00:00+00:00"},
       {"state", "done"},
       {"stype_in", "native"},
       {"stype_out", "product_id"},
       {"symbols", {"GEZ2", "GEH3"}},
       {"ts_expiration", "2022-11-30 15:29:03.010429+00:00"},
       {"ts_process_done", "2022-10-31 15:29:03.010429+00:00"},
       {"ts_process_start", "2022-10-31 15:29:01.104930+00:00"},
       {"ts_queued", "2022-10-31 15:28:58.933725+00:00"},
       {"ts_received", "2022-10-31 15:28:58.233520+00:00"},
       {"user_id", "A_USER"}}};
  mock_server_.MockGetJson("/v0/batch.list_jobs", kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target{kApiKey, "localhost",
                               static_cast<std::uint16_t>(port)};
  const auto res = target.BatchListJobs();
  ASSERT_EQ(res.size(), 2);
  const std::vector<std::string> symbols{"GEZ2", "GEH3"};
  EXPECT_EQ(res[1].symbols, symbols);
  EXPECT_EQ(res[0].ts_expiration, "2022-11-30 15:27:10.148788+00:00");
}

TEST_F(HistoricalTests, TestMetadataListPublishers) {
  const nlohmann::json kResp{
      {"GLBX", 1},
      {"XNAS", 2},
  };
  mock_server_.MockGetJson("/v0/metadata.list_publishers", kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target{kApiKey, "localhost",
                               static_cast<std::uint16_t>(port)};
  const auto res = target.MetadataListPublishers();
  EXPECT_EQ(res.size(), kResp.size());
  EXPECT_EQ(res.at("GLBX"), kResp.at("GLBX"));
  EXPECT_EQ(res.at("XNAS"), kResp.at("XNAS"));
}

TEST_F(HistoricalTests, TestMetadataListDatasets_Simple) {
  const nlohmann::json kResp{
      dataset::kGlbxMdp3,
      dataset::kXnasItch,
  };
  mock_server_.MockGetJson("/v0/metadata.list_datasets", kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target{kApiKey, "localhost",
                               static_cast<std::uint16_t>(port)};
  const auto res = target.MetadataListDatasets();
  EXPECT_EQ(res.size(), kResp.size());
  EXPECT_EQ(res[0], kResp[0]);
  EXPECT_EQ(res[1], kResp[1]);
}

TEST_F(HistoricalTests, TestMetadataListDatasets_Full) {
  const nlohmann::json kResp{dataset::kXnasItch};
  mock_server_.MockGetJson("/v0/metadata.list_datasets",
                           {{"start_date", "2021-01-05"}}, kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target{kApiKey, "localhost",
                               static_cast<std::uint16_t>(port)};
  const auto res = target.MetadataListDatasets("2021-01-05", "");
  EXPECT_EQ(res.size(), kResp.size());
  EXPECT_EQ(res[0], kResp[0]);
}

TEST_F(HistoricalTests, TestMetadataListSchemas_Simple) {
  const nlohmann::json kResp{"mbo",      "mbp-1",    "mbp-10",
                             "tbbo",     "trades",   "ohlcv-1s",
                             "ohlcv-1m", "ohlcv-1h", "ohlcv-1d"};
  mock_server_.MockGetJson("/v0/metadata.list_schemas",
                           {{"dataset", dataset::kGlbxMdp3}}, kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target{kApiKey, "localhost",
                               static_cast<std::uint16_t>(port)};
  const auto res = target.MetadataListSchemas(dataset::kGlbxMdp3);
  const std::vector<Schema> kExp{
      Schema::Mbo,     Schema::Mbp1,    Schema::Mbp10,
      Schema::Tbbo,    Schema::Trades,  Schema::Ohlcv1S,
      Schema::Ohlcv1M, Schema::Ohlcv1H, Schema::Ohlcv1D};
  ASSERT_EQ(res.size(), kResp.size());
  ASSERT_EQ(res.size(), kExp.size());
  for (std::size_t i = 0; i < res.size(); ++i) {
    EXPECT_EQ(res[i], kExp[i]) << "Index " << i;
  }
}

TEST_F(HistoricalTests, TestMetadataListSchemas_Full) {
  const nlohmann::json kResp{"mbo", "mbp-1", "ohlcv-1m", "ohlcv-1h",
                             "ohlcv-1d"};
  mock_server_.MockGetJson(
      "/v0/metadata.list_schemas",
      {{"dataset", dataset::kGlbxMdp3}, {"end_date", "2020-01-01"}}, kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target{kApiKey, "localhost",
                               static_cast<std::uint16_t>(port)};
  const auto res =
      target.MetadataListSchemas(dataset::kGlbxMdp3, {}, "2020-01-01");
  const std::vector<Schema> kExp{Schema::Mbo, Schema::Mbp1, Schema::Ohlcv1M,
                                 Schema::Ohlcv1H, Schema::Ohlcv1D};
  ASSERT_EQ(res.size(), kResp.size());
  ASSERT_EQ(res.size(), kExp.size());
  for (std::size_t i = 0; i < res.size(); ++i) {
    EXPECT_EQ(res[i], kExp[i]) << "Index " << i;
  }
}

TEST_F(HistoricalTests, TestMetadataListFields) {
  const nlohmann::json kResp{{dataset::kGlbxMdp3,
                              {{"dbz",
                                {{"trades",
                                  {{"length", "uint8_t"},
                                   {"rtype", "uint8_t"},
                                   {"dataset_id", "uint16_t"}}}}}}}};
  mock_server_.MockGetJson("/v0/metadata.list_fields",
                           {{"dataset", dataset::kGlbxMdp3},
                            {"encoding", "dbz"},
                            {"schema", "trades"}},
                           kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target{kApiKey, "localhost",
                               static_cast<std::uint16_t>(port)};
  const auto res = target.MetadataListFields(dataset::kGlbxMdp3, Encoding::Dbz,
                                             Schema::Trades);
  const FieldsByDatasetEncodingAndSchema kExp{
      {dataset::kGlbxMdp3,
       {{Encoding::Dbz,
         {{Schema::Trades,
           {{"length", "uint8_t"},
            {"rtype", "uint8_t"},
            {"dataset_id", "uint16_t"}}}}}}}};
  const auto& tradesRes =
      res.at(dataset::kGlbxMdp3).at(Encoding::Dbz).at(Schema::Trades);
  EXPECT_EQ(tradesRes.at("length"), "uint8_t");
  EXPECT_EQ(tradesRes.at("rtype"), "uint8_t");
  EXPECT_EQ(tradesRes.at("dataset_id"), "uint16_t");
}

TEST_F(HistoricalTests, TestMetadataListEncodings) {
  const nlohmann::json kResp{"dbz", "csv", "json"};
  mock_server_.MockGetJson("/v0/metadata.list_encodings", kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target{kApiKey, "localhost",
                               static_cast<std::uint16_t>(port)};
  const auto res = target.MetadataListEncodings();
  const std::vector<Encoding> kExp{Encoding::Dbz, Encoding::Csv,
                                   Encoding::Json};
  EXPECT_EQ(res, kExp);
}

TEST_F(HistoricalTests, TestMetadataListCompressions) {
  const nlohmann::json kResp{"none", "zstd"};
  mock_server_.MockGetJson("/v0/metadata.list_compressions", kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target{kApiKey, "localhost",
                               static_cast<std::uint16_t>(port)};
  const auto res = target.MetadataListCompressions();
  const std::vector<Compression> kExp{Compression::None, Compression::Zstd};
  EXPECT_EQ(res, kExp);
}

TEST_F(HistoricalTests, TestMetadataGetDatasetCondition) {
  const nlohmann::json kResp{
      {"condition", "bad"},
      {"details",
       {{{"date", "2022-11-07"}, {"condition", "available"}},
        {{"date", "2022-11-08"}, {"condition", "bad"}},
        {{"date", "2022-11-09"}, {"condition", "bad"}},
        {{"date", "2022-11-10"}, {"condition", "available"}}}},
      {"adjusted_start_date", "2022-11-07"},
      {"adjusted_end_date", "2022-11-10"}};
  mock_server_.MockGetJson("/v0/metadata.get_dataset_condition",
                           {{"dataset", dataset::kXnasItch},
                            {"start_date", "2022-11-06"},
                            {"end_date", "2022-11-10"}},
                           kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target{kApiKey, "localhost",
                               static_cast<std::uint16_t>(port)};
  const auto res = target.MetadataGetDatasetCondition(
      dataset::kXnasItch, "2022-11-06", "2022-11-10");
  EXPECT_EQ(res.condition, DatasetCondition::Bad);
  ASSERT_EQ(res.details.size(), 4);
  EXPECT_EQ(res.details[0].date, res.adjusted_start_date);
  EXPECT_EQ(res.details[1].date, "2022-11-08");
  EXPECT_EQ(res.details[2].date, "2022-11-09");
  EXPECT_EQ(res.details[3].date, res.adjusted_end_date);
  EXPECT_EQ(res.details[0].condition, DatasetCondition::Available);
  EXPECT_EQ(res.details[1].condition, DatasetCondition::Bad);
  EXPECT_EQ(res.details[2].condition, DatasetCondition::Bad);
  EXPECT_EQ(res.details[3].condition, DatasetCondition::Available);
  EXPECT_EQ(res.adjusted_start_date, "2022-11-07");
  EXPECT_EQ(res.adjusted_end_date, "2022-11-10");
}

TEST_F(HistoricalTests, TestMetadataListUnitPrices_Dataset) {
  const nlohmann::json kResp{
      {"historical-streaming",
       {{"mbo", 21.05}, {"mbp-1", 82.05}, {"status", 62.72}}}};
  mock_server_.MockGetJson("/v0/metadata.list_unit_prices",
                           {{"dataset", dataset::kGlbxMdp3}}, kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target{kApiKey, "localhost",
                               static_cast<std::uint16_t>(port)};
  const auto res = target.MetadataListUnitPrices(dataset::kGlbxMdp3);
  const std::map<Schema, double> kExp{
      {Schema::Mbo, 21.05}, {Schema::Mbp1, 82.05}, {Schema::Status, 62.72}};
  ASSERT_EQ(res.size(), 1);
  const auto& hist_streaming_res = res.at(FeedMode::HistoricalStreaming);
  ASSERT_EQ(hist_streaming_res.size(), kExp.size());
  for (const auto& schema_and_price : kExp) {
    EXPECT_DOUBLE_EQ(schema_and_price.second, kExp.at(schema_and_price.first))
        << "Key " << ToString(schema_and_price.first);
  }
}

TEST_F(HistoricalTests, TestMetadataListUnitPrices_FeedMode) {
  const nlohmann::json kResp{
      {"historical-streaming",
       {{"mbo", 21.05}, {"mbp-1", 82.05}, {"status", 62.72}}}};
  mock_server_.MockGetJson(
      "/v0/metadata.list_unit_prices",
      {{"dataset", dataset::kGlbxMdp3}, {"mode", "historical-streaming"}},
      kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target{kApiKey, "localhost",
                               static_cast<std::uint16_t>(port)};
  const auto res = target.MetadataListUnitPrices(dataset::kGlbxMdp3,
                                                 FeedMode::HistoricalStreaming);
  const std::map<Schema, double> kExp{
      {Schema::Mbo, 21.05}, {Schema::Mbp1, 82.05}, {Schema::Status, 62.72}};
  ASSERT_EQ(res.size(), kExp.size());
  for (const auto& schema_and_price : kExp) {
    EXPECT_DOUBLE_EQ(schema_and_price.second, kExp.at(schema_and_price.first))
        << "Key " << ToString(schema_and_price.first);
  }
}

TEST_F(HistoricalTests, TestMetadataListUnitPrices_Schema) {
  const nlohmann::json kResp{{"historical-streaming", {{"mbo", 21.05}}},
                             {"historical", {{"mbo", 19.95}}},
                             {"live", {{"mbo", 43.14}}}};
  mock_server_.MockGetJson("/v0/metadata.list_unit_prices",
                           {{"dataset", dataset::kGlbxMdp3}, {"schema", "mbo"}},
                           kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target{kApiKey, "localhost",
                               static_cast<std::uint16_t>(port)};
  const auto res =
      target.MetadataListUnitPrices(dataset::kGlbxMdp3, Schema::Mbo);
  const std::map<FeedMode, double> kExp{{FeedMode::HistoricalStreaming, 21.05},
                                        {FeedMode::Historical, 19.95},
                                        {FeedMode::Live, 43.14}};
  ASSERT_EQ(res.size(), kExp.size());
  for (const auto& mode_and_price : kExp) {
    EXPECT_DOUBLE_EQ(mode_and_price.second, kExp.at(mode_and_price.first))
        << "Key " << ToString(mode_and_price.first);
  }
}

TEST_F(HistoricalTests, TestMetadataListUnitPrices_FullySpecified) {
  const nlohmann::json kResp = 43.21;
  mock_server_.MockGetJson(
      "/v0/metadata.list_unit_prices",
      {{"dataset", dataset::kGlbxMdp3}, {"schema", "mbo"}, {"mode", "live"}},
      kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target{kApiKey, "localhost",
                               static_cast<std::uint16_t>(port)};
  const auto res = target.MetadataListUnitPrices(dataset::kGlbxMdp3,
                                                 FeedMode::Live, Schema::Mbo);
  EXPECT_DOUBLE_EQ(res, 43.21);
}

TEST_F(HistoricalTests, TestMetadataGetBillableSize_Simple) {
  const nlohmann::json kResp = 44688;
  mock_server_.MockGetJson("/v0/metadata.get_billable_size",
                           {{"dataset", dataset::kGlbxMdp3},
                            {"start", "2020-06-06T00:00"},
                            {"end", "2021-03-02T00:00"},
                            {"schema", "trades"}},
                           kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target{kApiKey, "localhost",
                               static_cast<std::uint16_t>(port)};
  const auto res = target.MetadataGetBillableSize(
      dataset::kGlbxMdp3, "2020-06-06T00:00", "2021-03-02T00:00", kAllSymbols,
      Schema::Trades);
  ASSERT_EQ(res, 44688);
}

TEST_F(HistoricalTests, TestMetadataGetBillableSize_Full) {
  const nlohmann::json kResp = 55238;
  mock_server_.MockGetJson("/v0/metadata.get_billable_size",
                           {{"dataset", dataset::kGlbxMdp3},
                            {"start", "2020-06-06T00:00"},
                            {"end", "2021-03-02T00:00"},
                            {"symbols", "NG,LNQ"},
                            {"schema", "tbbo"},
                            {"stype_in", "smart"}},
                           kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target{kApiKey, "localhost",
                               static_cast<std::uint16_t>(port)};
  const auto res = target.MetadataGetBillableSize(
      dataset::kGlbxMdp3, "2020-06-06T00:00", "2021-03-02T00:00", {"NG", "LNQ"},
      Schema::Tbbo, SType::Smart, {});
  ASSERT_EQ(res, 55238);
}

TEST_F(HistoricalTests, TestMetadataGetCost_Simple) {
  const nlohmann::json kResp = 0.65783;
  mock_server_.MockGetJson("/v0/metadata.get_cost",
                           {{"dataset", dataset::kGlbxMdp3},
                            {"start", "2020-06-06T00:00"},
                            {"end", "2021-03-02T00:00"},
                            {"symbols", "MESN1,MESQ1"},
                            {"schema", "trades"}},
                           kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target{kApiKey, "localhost",
                               static_cast<std::uint16_t>(port)};
  const auto res = target.MetadataGetCost(
      dataset::kGlbxMdp3, "2020-06-06T00:00", "2021-03-02T00:00",
      {"MESN1", "MESQ1"}, Schema::Trades);
  ASSERT_DOUBLE_EQ(res, 0.65783);
}

TEST_F(HistoricalTests, TestMetadataGetCost_Full) {
  const nlohmann::json kResp = 0.714;
  mock_server_.MockGetJson("/v0/metadata.get_cost",
                           {{"dataset", dataset::kGlbxMdp3},
                            {"start", "2020-06-06T00:00"},
                            {"end", "2021-03-02T00:00"},
                            {"mode", "historical-streaming"},
                            {"symbols", "MES,SPY"},
                            {"schema", "tbbo"},
                            {"stype_in", "smart"}},
                           kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target{kApiKey, "localhost",
                               static_cast<std::uint16_t>(port)};
  const auto res =
      target.MetadataGetCost(dataset::kGlbxMdp3, "2020-06-06T00:00",
                             "2021-03-02T00:00", {"MES", "SPY"}, Schema::Tbbo,
                             FeedMode::HistoricalStreaming, SType::Smart, {});
  ASSERT_DOUBLE_EQ(res, 0.714);
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
      {"stype_in", "native"},
      {"stype_out", "product_id"},
      {"start_date", "2022-06-06"},
      {"end_date", "2022-06-10"},
      {"partial", nlohmann::json::array()},
      {"not_found", nlohmann::json::array()},
      {"message", "OK"},
      {"status", 0},
  };

  mock_server_.MockGetJson("/v0/symbology.resolve",
                           {
                               {"dataset", dataset::kGlbxMdp3},
                               {"start_date", "2022-06-06"},
                               {"end_date", "2022-06-10"},
                               {"symbols", "ESM2"},
                               {"stype_in", "native"},
                               {"stype_out", "product_id"},
                           },
                           kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target{kApiKey, "localhost",
                               static_cast<std::uint16_t>(port)};
  const auto res =
      target.SymbologyResolve(dataset::kGlbxMdp3, "2022-06-06", "2022-06-10",
                              {"ESM2"}, SType::Native, SType::ProductId);
  EXPECT_TRUE(res.not_found.empty());
  EXPECT_TRUE(res.partial.empty());
  ASSERT_EQ(res.mappings.size(), 1);
  const auto& esm2_mappings = res.mappings.at("ESM2");
  ASSERT_EQ(esm2_mappings.size(), 1);
  const auto& esm2_mapping = esm2_mappings.at(0);
  EXPECT_EQ(esm2_mapping.start_date, "2022-06-06");
  EXPECT_EQ(esm2_mapping.end_date, "2022-06-10");
  EXPECT_EQ(esm2_mapping.symbol, "3403");
}

TEST_F(HistoricalTests, TestTimeseriesStream_Basic) {
  mock_server_.MockStreamDbz("/v0/timeseries.stream",
                             {{"dataset", dataset::kGlbxMdp3},
                              {"symbols", "ESH1"},
                              {"schema", "mbo"},
                              {"start", "1609160400000711344"},
                              {"end", "1609160800000711344"},
                              {"encoding", "dbz"},
                              {"stype_in", "native"},
                              {"stype_out", "product_id"},
                              {"limit", "2"}},
                             TEST_BUILD_DIR "/data/test_data.mbo.dbz");
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target{kApiKey, "localhost",
                               static_cast<std::uint16_t>(port)};
  std::unique_ptr<Metadata> metadata_ptr;
  std::vector<MboMsg> mbo_records;
  target.TimeseriesStream(
      dataset::kGlbxMdp3,
      UnixNanos{std::chrono::nanoseconds{1609160400000711344}},
      UnixNanos{std::chrono::nanoseconds{1609160800000711344}}, {"ESH1"},
      Schema::Mbo, SType::Native, SType::ProductId, 2,
      [&metadata_ptr](Metadata&& metadata) {
        // no std::make_unique until C++14
        metadata_ptr =
            std::unique_ptr<Metadata>{new Metadata(std::move(metadata))};
      },
      [&mbo_records](const Record& record) {
        mbo_records.emplace_back(record.Get<MboMsg>());
        return KeepGoing::Continue;
      });
  EXPECT_EQ(metadata_ptr->record_count, 2);
  EXPECT_EQ(metadata_ptr->limit, 2);
  EXPECT_EQ(metadata_ptr->schema, Schema::Mbo);
  EXPECT_EQ(mbo_records.size(), 2);
}

TEST_F(HistoricalTests, TestTimeseriesStream_NoMetadataCallback) {
  mock_server_.MockStreamDbz("/v0/timeseries.stream",
                             {{"dataset", dataset::kGlbxMdp3},
                              {"start", "2022-10-21T13:30"},
                              {"end", "2022-10-21T20:00"},
                              {"symbols", "CYZ2"},
                              {"schema", "tbbo"},
                              {"encoding", "dbz"},
                              {"stype_in", "native"},
                              {"stype_out", "product_id"}},
                             TEST_BUILD_DIR "/data/test_data.tbbo.dbz");
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target{kApiKey, "localhost",
                               static_cast<std::uint16_t>(port)};
  std::vector<TbboMsg> mbo_records;
  target.TimeseriesStream(dataset::kGlbxMdp3, "2022-10-21T13:30",
                          "2022-10-21T20:00", {"CYZ2"}, Schema::Tbbo,
                          [&mbo_records](const Record& record) {
                            mbo_records.emplace_back(record.Get<TbboMsg>());
                            return KeepGoing::Continue;
                          });
  EXPECT_EQ(mbo_records.size(), 2);
}

// should get helpful message if there's a problem with the request
TEST_F(HistoricalTests, TestTimeseriesStream_BadRequest) {
  const nlohmann::json resp{
      {"detail", "Authorization failed: illegal chars in username."}};
  mock_server_.MockBadRequest("/v0/timeseries.stream", resp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target{kApiKey, "localhost",
                               static_cast<std::uint16_t>(port)};
  try {
    target.TimeseriesStream(
        dataset::kGlbxMdp3,
        UnixNanos{std::chrono::nanoseconds{1609160400000711344}},
        UnixNanos{std::chrono::nanoseconds{1609160800000711344}}, {"E5"},
        Schema::Mbo, SType::Smart, SType::ProductId, 2, [](Metadata&&) {},
        [](const Record&) { return KeepGoing::Continue; });
    FAIL() << "Call to TimeseriesStream was supposed to throw";
  } catch (const std::exception& exc) {
    ASSERT_STREQ(
        exc.what(),
        "Received an error response from request to "
        "/v0/timeseries.stream with status 400 and body "
        "'{\"detail\":\"Authorization failed: illegal chars in username.\"}'"

    );
  }
}

TEST_F(HistoricalTests, TestTimeseriesStream_CallbackException) {
  mock_server_.MockStreamDbz("/v0/timeseries.stream", {},
                             TEST_BUILD_DIR "/data/test_data.mbo.dbz");
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target{kApiKey, "localhost",
                               static_cast<std::uint16_t>(port)};
  ASSERT_THROW(target.TimeseriesStream(
                   dataset::kGlbxMdp3,
                   UnixNanos{std::chrono::nanoseconds{1609160400000711344}},
                   UnixNanos{std::chrono::nanoseconds{1609160800000711344}},
                   {"ESH1"}, Schema::Mbo, SType::Native, SType::ProductId, 2,
                   [](Metadata&&) { throw std::logic_error{"Test failure"}; },
                   [](const Record&) { return KeepGoing::Continue; }),
               std::logic_error);
}

TEST_F(HistoricalTests, TestTimeseriesStreamCancellation) {
  mock_server_.MockStreamDbz("/v0/timeseries.stream", {},
                             TEST_BUILD_DIR "/data/test_data.mbo.dbz");
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target{kApiKey, "localhost",
                               static_cast<std::uint16_t>(port)};
  std::uint32_t call_count = 0;
  target.TimeseriesStream(
      dataset::kGlbxMdp3,
      UnixNanos{std::chrono::nanoseconds{1609160400000711344}},
      UnixNanos{std::chrono::nanoseconds{1609160800000711344}}, {"ESH1"},
      Schema::Mbo, SType::Native, SType::ProductId, 2, [](Metadata&&) {},
      [&call_count](const Record&) {
        ++call_count;
        return KeepGoing::Stop;
      });
  // Should gracefully exit after first record, even though there are two
  // records in the file
  ASSERT_EQ(call_count, 1);
}

TEST_F(HistoricalTests, TestTimeseriesStreamToFile) {
  mock_server_.MockStreamDbz("/v0/timeseries.stream",
                             {{"dataset", dataset::kGlbxMdp3},
                              {"start", "2022-10-21T13:30"},
                              {"end", "2022-10-21T20:00"},
                              {"symbols", "CYZ2"},
                              {"schema", "tbbo"},
                              {"encoding", "dbz"},
                              {"stype_in", "native"},
                              {"stype_out", "product_id"}},
                             TEST_BUILD_DIR "/data/test_data.tbbo.dbz");
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target{kApiKey, "localhost",
                               static_cast<std::uint16_t>(port)};
  const TempFile temp_file{testing::TempDir() + "/" + __FUNCTION__};
  target.TimeseriesStreamToFile(dataset::kGlbxMdp3, "2022-10-21T13:30",
                                "2022-10-21T20:00", {"CYZ2"}, Schema::Tbbo,
                                temp_file.Path());
  // running it a second time should overwrite previous data
  FileBento bento = target.TimeseriesStreamToFile(
      dataset::kGlbxMdp3, "2022-10-21T13:30", "2022-10-21T20:00", {"CYZ2"},
      Schema::Tbbo, temp_file.Path());
  std::size_t counter{};
  bento.Replay([&counter](const Record&) {
    ++counter;
    return KeepGoing::Continue;
  });
  ASSERT_EQ(counter, 2);
}

TEST(JsonImplementationTests,
     TestParsingNumberNotPreciselyRepresentableAsDouble) {
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
  ASSERT_THROW(databento::HistoricalBuilder().SetKeyFromEnv().Build(),
               Exception);
}
}  // namespace test
}  // namespace databento
