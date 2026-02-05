#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include "databento/compat.hpp"
#include "databento/detail/buffer.hpp"
#include "databento/detail/zstd_stream.hpp"
#include "databento/enums.hpp"
#include "databento/file_stream.hpp"

namespace databento::detail::tests {
TEST(ZstdStreamTests, TestMultiFrameFiles) {
  constexpr auto kRecordCount = 8;
  const std::string file_path = TEST_DATA_DIR "/multi-frame.definition.v1.dbn.frag.zst";

  databento::detail::ZstdDecodeStream target{
      std::make_unique<databento::InFileStream>(file_path)};
  for (std::size_t i = 0; i < kRecordCount; ++i) {
    databento::InstrumentDefMsgV1 def_msg;
    target.ReadExact(reinterpret_cast<std::byte*>(&def_msg), sizeof(def_msg));
    EXPECT_EQ(def_msg.hd.rtype, databento::RType::InstrumentDef);
  }
}

TEST(ZstdStreamTests, TestIdentity) {
  std::vector<std::int64_t> source_data;
  for (std::int64_t i = 0; i < 100000; ++i) {
    source_data.emplace_back(i);
  }
  auto size = source_data.size() * sizeof(std::int64_t);
  detail::Buffer mock_io;
  {
    ZstdCompressStream compressor{&mock_io};
    for (auto it = source_data.begin(); it != source_data.end(); it += 100) {
      compressor.WriteAll(reinterpret_cast<const std::byte*>(&*it),
                          100 * sizeof(std::int64_t));
    }
  }
  std::vector<std::byte> res(size);
  ZstdDecodeStream decode{std::make_unique<detail::Buffer>(std::move(mock_io))

  };
  decode.ReadExact(res.data(), size);
}

TEST(ZstdStreamTests, TestFlush) {
  // Test that Flush() makes data available for reading without ending the stream
  const std::string kTestData = "DBN\x01\x00\x00\x00TestData123";
  detail::Buffer mock_io;
  {
    ZstdCompressStream compressor{&mock_io};
    // Write small data that would normally be buffered
    compressor.WriteAll(reinterpret_cast<const std::byte*>(kTestData.data()),
                        kTestData.size());
    compressor.Flush();
    // At this point, data should be in mock_io even though compressor isn't destroyed

    // Write more data after flush
    compressor.WriteAll(reinterpret_cast<const std::byte*>(kTestData.data()),
                        kTestData.size());
    compressor.Flush();
  }

  // Verify we can decode both chunks
  std::vector<std::byte> res(kTestData.size() * 2);
  ZstdDecodeStream decode{std::make_unique<detail::Buffer>(std::move(mock_io))};
  decode.ReadExact(res.data(), kTestData.size() * 2);

  std::string result(reinterpret_cast<const char*>(res.data()), res.size());
  EXPECT_EQ(result, kTestData + kTestData);
}

// Mock IReadable that always returns a timeout
class TimeoutReader : public IReadable {
 public:
  void ReadExact(std::byte*, std::size_t) override {
    throw std::runtime_error{"TimeoutReader does not support ReadExact"};
  }
  std::size_t ReadSome(std::byte*, std::size_t) override { return 0; }
  Result ReadSome(std::byte*, std::size_t, std::chrono::milliseconds) override {
    return {0, Status::Timeout};
  }
};

TEST(ZstdStreamTests, TestDecodeTimeout) {
  ZstdDecodeStream target{std::make_unique<TimeoutReader>()};

  std::vector<std::byte> buffer(100);
  auto result =
      target.ReadSome(buffer.data(), buffer.size(), std::chrono::milliseconds{100});

  EXPECT_EQ(result.read_size, 0);
  EXPECT_EQ(result.status, IReadable::Status::Timeout);
}
}  // namespace databento::detail::tests
