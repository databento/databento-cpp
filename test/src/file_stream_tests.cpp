#include <gtest/gtest.h>

#include "databento/detail/file_stream.hpp"
#include "databento/exceptions.hpp"

namespace databento {
namespace detail {
namespace test {
TEST(FileStreamTests, TestReadExactInsufficient) {
  const std::string file_path = TEST_BUILD_DIR "/data/test_data.ohlcv-1d.dbn";
  databento::detail::FileStream target{file_path};
  std::vector<std::uint8_t> buffer(1024);  // File is less than 1KiB
  try {
    target.ReadExact(buffer.data(), buffer.size());
    FAIL() << "Expected throw";
  } catch (const databento::Exception& exc) {
    ASSERT_STREQ(exc.what(),
                 "Unexpected end of file, expected 1024 bytes, got 206");
  }
}

TEST(FileStreamTests, TestReadSomeLessThanMax) {
  const std::string file_path = TEST_BUILD_DIR "/data/test_data.ohlcv-1d.dbn";
  databento::detail::FileStream target{file_path};
  std::vector<std::uint8_t> buffer(1024);  // File is less than 1KiB
  const auto read_size = target.ReadSome(buffer.data(), buffer.size());
  ASSERT_GT(read_size, 0);
  ASSERT_TRUE(std::any_of(buffer.cbegin(), buffer.cend(),
                          [](std::uint8_t byte) { return byte != 0; }));
}
}  // namespace test
}  // namespace detail
}  // namespace databento
