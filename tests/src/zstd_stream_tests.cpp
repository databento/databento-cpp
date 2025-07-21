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
}  // namespace databento::detail::tests
