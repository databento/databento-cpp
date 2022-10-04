#include <gtest/gtest.h>

#include <thread>
// ignore warnings from httplib
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wformat-nonliteral"
#include <httplib.h>
#pragma clang diagnostic pop

#include <algorithm>
#include <cstdlib>
#include <nlohmann/json_fwd.hpp>
#include <stdexcept>

#include "databento/enums.hpp"
#include "databento/historical.hpp"
#include "mock/mock_server.hpp"

namespace databento {
namespace test {
constexpr auto kApiKey = "HIST_SECRET";

class HistoricalTests : public ::testing::Test {
 protected:
  mock::MockServer mock_server_{kApiKey};
};

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
      "GLBX.MDP3",
      "XNAS.ITCH",
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
  const nlohmann::json kResp{
      "XNAS.ITCH",
  };
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
                           {{"dataset", "GLBX.MDP3"}}, kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target{kApiKey, "localhost",
                               static_cast<std::uint16_t>(port)};
  const auto res = target.MetadataListSchemas("GLBX.MDP3");
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
      {{"dataset", "GLBX.MDP3"}, {"end_date", "2020-01-01"}}, kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target{kApiKey, "localhost",
                               static_cast<std::uint16_t>(port)};
  const auto res = target.MetadataListSchemas("GLBX.MDP3", {}, "2020-01-01");
  const std::vector<Schema> kExp{Schema::Mbo, Schema::Mbp1, Schema::Ohlcv1M,
                                 Schema::Ohlcv1H, Schema::Ohlcv1D};
  ASSERT_EQ(res.size(), kResp.size());
  ASSERT_EQ(res.size(), kExp.size());
  for (std::size_t i = 0; i < res.size(); ++i) {
    EXPECT_EQ(res[i], kExp[i]) << "Index " << i;
  }
}

TEST_F(HistoricalTests, TestMetadataListUnitPrices_Dataset) {
  const nlohmann::json kResp{
      {"historical-streaming",
       {{"mbo", 21.05}, {"mbp-1", 82.05}, {"status", 62.72}}}};
  mock_server_.MockGetJson("/v0/metadata.list_unit_prices",
                           {{"dataset", "GLBX.MDP3"}}, kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target{kApiKey, "localhost",
                               static_cast<std::uint16_t>(port)};
  const auto res = target.MetadataListUnitPrices("GLBX.MDP3");
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
      {{"dataset", "GLBX.MDP3"}, {"mode", "historical-streaming"}}, kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target{kApiKey, "localhost",
                               static_cast<std::uint16_t>(port)};
  const auto res =
      target.MetadataListUnitPrices("GLBX.MDP3", FeedMode::HistoricalStreaming);
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
                           {{"dataset", "GLBX.MDP3"}, {"schema", "mbo"}},
                           kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target{kApiKey, "localhost",
                               static_cast<std::uint16_t>(port)};
  const auto res = target.MetadataListUnitPrices("GLBX.MDP3", Schema::Mbo);
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
      {{"dataset", "GLBX.MDP3"}, {"schema", "mbo"}, {"mode", "live"}}, kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target{kApiKey, "localhost",
                               static_cast<std::uint16_t>(port)};
  const auto res =
      target.MetadataListUnitPrices("GLBX.MDP3", FeedMode::Live, Schema::Mbo);
  EXPECT_DOUBLE_EQ(res, 43.21);
}

TEST_F(HistoricalTests, TestMetadataGetBillableSize_Simple) {
  const nlohmann::json kResp = 44688;
  mock_server_.MockGetJson("/v0/metadata.get_billable_size",
                           {{"dataset", "GLBX.MDP3"},
                            {"start", "2020-06-06T00:00"},
                            {"end", "2021-03-02T00:00"}},
                           kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target{kApiKey, "localhost",
                               static_cast<std::uint16_t>(port)};
  const auto res = target.MetadataGetBillableSize(
      "GLBX.MDP3", "2020-06-06T00:00", "2021-03-02T00:00");
  ASSERT_EQ(res, 44688);
}

TEST_F(HistoricalTests, TestMetadataGetBillableSize_Full) {
  const nlohmann::json kResp = 55238;
  mock_server_.MockGetJson("/v0/metadata.get_billable_size",
                           {{"dataset", "GLBX.MDP3"},
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
      "GLBX.MDP3", "2020-06-06T00:00", "2021-03-02T00:00", {"NG", "LNQ"},
      Schema::Tbbo, SType::Smart, {});
  ASSERT_EQ(res, 55238);
}

TEST_F(HistoricalTests, TestMetadataGetCost_Simple) {
  const nlohmann::json kResp = 0.65783;
  mock_server_.MockGetJson("/v0/metadata.get_cost",
                           {{"dataset", "GLBX.MDP3"},
                            {"start", "2020-06-06T00:00"},
                            {"end", "2021-03-02T00:00"}},
                           kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target{kApiKey, "localhost",
                               static_cast<std::uint16_t>(port)};
  const auto res = target.MetadataGetCost("GLBX.MDP3", "2020-06-06T00:00",
                                          "2021-03-02T00:00");
  ASSERT_DOUBLE_EQ(res, 0.65783);
}

TEST_F(HistoricalTests, TestMetadataGetCost_Full) {
  const nlohmann::json kResp = 0.714;
  mock_server_.MockGetJson("/v0/metadata.get_cost",
                           {{"dataset", "GLBX.MDP3"},
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
      target.MetadataGetCost("GLBX.MDP3", "2020-06-06T00:00",
                             "2021-03-02T00:00", FeedMode::HistoricalStreaming,
                             {"MES", "SPY"}, Schema::Tbbo, SType::Smart, {});
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
                               {"dataset", "GLBX.MDP3"},
                               {"symbols", "ESM2"},
                               {"stype_in", "native"},
                               {"stype_out", "product_id"},
                               {"start_date", "2022-06-06"},
                               {"end_date", "2022-06-10"},

                           },
                           kResp);
  const auto port = mock_server_.ListenOnThread();

  databento::Historical target{kApiKey, "localhost",
                               static_cast<std::uint16_t>(port)};
  const auto res =
      target.SymbologyResolve("GLBX.MDP3", {"ESM2"}, SType::Native,
                              SType::ProductId, "2022-06-06", "2022-06-10");
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

TEST(JsonImplementationTests, TestParsingNumberNotPreciselyRepresentableAsDouble) {
  auto const number_json =  nlohmann::json::parse("1609160400000711344");
  EXPECT_TRUE(number_json.is_number());
  EXPECT_EQ(number_json, 1609160400000711344);
}

TEST(HistoricalBuilderTests, TestBasic) {
  constexpr auto kKey = "SECRET";

  const auto client = databento::HistoricalBuilder()
                          .key(kKey)
                          .gateway(databento::HistoricalGateway::Bo1)
                          .Build();
  EXPECT_EQ(client.key(), kKey);
  EXPECT_EQ(client.gateway(), "https://hist.databento.com");
}

TEST(HistoricalBuilderTests, TestMissingKey) {
  ASSERT_THROW(databento::HistoricalBuilder().Build(), std::logic_error);
}

TEST(HistoricalBuilderTests, TestkeyFromEnv) {
  constexpr auto kKey = "SECRET_KEY";
  ASSERT_EQ(::setenv("DATABENTO_API_KEY", kKey, 1), 0)
      << "Failed to set environment variable";
  const auto client = databento::HistoricalBuilder().keyFromEnv().Build();
  EXPECT_EQ(client.key(), kKey);
  EXPECT_EQ(client.gateway(), "https://hist.databento.com");
  // unsetting prevents this test from affecting others
  ASSERT_EQ(::unsetenv("DATABENTO_API_KEY"), 0)
      << "Failed to unset environment variable";
}

TEST(HistoricalBuilderTests, TestkeyFromEnvMissing) {
  ASSERT_THROW(databento::HistoricalBuilder().keyFromEnv().Build(),
               std::runtime_error);
}
}  // namespace test
}  // namespace databento
