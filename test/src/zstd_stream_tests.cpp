#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include "databento/compat.hpp"
#include "databento/detail/zstd_stream.hpp"
#include "databento/enums.hpp"
#include "databento/file_stream.hpp"
#include "databento/ireadable.hpp"
#include "mock/mock_io.hpp"

namespace databento {
namespace detail {
namespace test {
TEST(ZstdStreamTests, TestMultiFrameFiles) {
  constexpr auto kRecordCount = 8;
  const std::string file_path =
      TEST_BUILD_DIR "/data/multi-frame.definition.v1.dbn.zst";

  databento::detail::ZstdDecodeStream target{
      std::unique_ptr<databento::IReadable>{
          new databento::InFileStream{file_path}}};
  for (std::size_t i = 0; i < kRecordCount; ++i) {
    databento::InstrumentDefMsgV1 def_msg;
    target.ReadExact(reinterpret_cast<std::uint8_t*>(&def_msg),
                     sizeof(def_msg));
    EXPECT_EQ(def_msg.hd.rtype, databento::rtype::InstrumentDef);
  }
}

TEST(ZstdStreamTests, TestIdentity) {
  std::vector<std::int64_t> source_data;
  for (std::int64_t i = 0; i < 100000; ++i) {
    source_data.emplace_back(i);
  }
  auto size = source_data.size() * sizeof(std::int64_t);
  databento::test::mock::MockIo mock_io;
  {
    ZstdCompressStream compressor{&mock_io};
    for (auto it = source_data.begin(); it != source_data.end(); it += 100) {
      compressor.WriteAll(reinterpret_cast<const std::uint8_t*>(&*it),
                          100 * sizeof(std::int64_t));
    }
  }
  std::vector<std::uint8_t> res(size);
  ZstdDecodeStream decode{std::unique_ptr<IReadable>{
      new databento::test::mock::MockIo{std::move(mock_io)}}};
  decode.ReadExact(res.data(), size);
}
}  // namespace test
}  // namespace detail
}  // namespace databento
