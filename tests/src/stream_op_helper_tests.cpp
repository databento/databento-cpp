#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <sstream>
#include <vector>

#include "databento/constants.hpp"
#include "databento/enums.hpp"
#include "stream_op_helper.hpp"

namespace databento::tests {
TEST(StreamOpHelperTests, TestEmpty) {
  std::ostringstream stream;

  StreamOpBuilder{stream}.SetTypeName("EmptyT").SetSpacer(" ").Build().Finish();
  ASSERT_EQ(stream.str(), "EmptyT {}");
}

TEST(StreamOpHelperTests, TestSingleLine) {
  std::ostringstream stream;
  auto target = StreamOpBuilder{stream}.SetTypeName("TestClass").SetSpacer(" ").Build();
  target.AddField("schema", Schema::Ohlcv1D)
      .AddField("dataset", std::string{dataset::kXnasItch})
      .AddField("size", 10)
      .AddField("i8", static_cast<std::int8_t>(-10))
      .AddField("u8", static_cast<std::uint8_t>(16))
      .Finish();
  EXPECT_EQ(stream.str(),
            "TestClass { schema = ohlcv-1d, dataset = \"XNAS.ITCH\", size = "
            "10, i8 = -10, u8 = 16 }");
}

TEST(StreamOpHelperTests, TestMultiLine) {
  std::ostringstream stream;
  auto target =
      StreamOpBuilder{stream}.SetTypeName("TestClass").SetSpacer("\n    ").Build();
  target.AddField("schema", Schema::Ohlcv1D)
      .AddField("dataset", std::string{dataset::kXnasItch})
      .AddField("size", 10)
      .AddField("is_full", true)
      .AddField("action", 'A')
      .Finish();
  EXPECT_EQ(stream.str(),
            R"(TestClass {
    schema = ohlcv-1d,
    dataset = "XNAS.ITCH",
    size = 10,
    is_full = true,
    action = 'A'
})");
}

TEST(StreamOpHelperTests, TestWithVector) {
  const std::vector<Schema> test_data{Schema::Ohlcv1D, Schema::Mbp10, Schema::Ohlcv1M};

  std::ostringstream stream;
  auto target = StreamOpBuilder{stream}.SetSpacer(" ").Build();
  for (const auto schema : test_data) {
    target.AddItem(schema);
  }
  target.Finish();
  ASSERT_EQ(stream.str(), "{ ohlcv-1d, mbp-10, ohlcv-1m }");
}

TEST(StreamOpHelperTests, TestIndent) {
  const std::vector<Encoding> test_data{Encoding::Csv, Encoding::Dbn, Encoding::Json};

  std::ostringstream stream;
  auto target = StreamOpBuilder{stream}.SetSpacer("\n    ").SetIndent("    ").Build();
  for (const auto schema : test_data) {
    target.AddItem(schema);
  }
  target.Finish();
  ASSERT_EQ(stream.str(), R"({
        csv,
        dbn,
        json
    })");
}

TEST(StreamOpHelperTests, TestCharArray) {
  const std::array<char, 5> test_data{'U', 'S', 'D'};

  std::ostringstream stream;
  auto target = StreamOpBuilder{stream}.Build();
  target.AddField("array", test_data).Finish();
  ASSERT_EQ(stream.str(), R"({array = "USD"})");
}
}  // namespace databento::tests
