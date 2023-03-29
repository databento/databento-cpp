#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <memory>

#include "databento/detail/file_stream.hpp"
#include "databento/detail/zstd_stream.hpp"
#include "databento/enums.hpp"
#include "databento/ireadable.hpp"
#include "databento/record.hpp"

namespace databento {
namespace detail {
namespace test {
TEST(ZstdStreamTests, TestMultiFrameFiles) {
  constexpr auto kRecordCount = 8;
  const std::string file_path =
      TEST_BUILD_DIR "/data/multi-frame.definition.zst";

  databento::detail::ZstdStream target{std::unique_ptr<databento::IReadable>{
      new databento::detail::FileStream{file_path}}};
  for (std::size_t i = 0; i < kRecordCount; ++i) {
    databento::InstrumentDefMsg def_msg;
    target.ReadExact(reinterpret_cast<std::uint8_t*>(&def_msg),
                     sizeof(def_msg));
    EXPECT_EQ(def_msg.hd.rtype, databento::rtype::InstrumentDef);
  }
}
}  // namespace test
}  // namespace detail
}  // namespace databento
