#include <gtest/gtest.h>

#include <cstddef>
#include <filesystem>

#include "databento/exceptions.hpp"
#include "databento/file_stream.hpp"
#include "temp_file.hpp"

namespace databento::tests {
TEST(InFileStreamTests, TestReadExactInsufficient) {
  const std::string file_path = TEST_DATA_DIR "/test_data.mbo.v3.dbn";
  InFileStream target{file_path};
  std::vector<std::byte> buffer(1024);  // File is less than 1KiB
  try {
    target.ReadExact(buffer.data(), buffer.size());
    FAIL() << "Expected throw";
  } catch (const databento::Exception& exc) {
    ASSERT_STREQ(exc.what(), "Unexpected end of file, expected 1024 bytes, got 472");
  }
}

TEST(InFileStreamTests, TestReadSomeLessThanMax) {
  const std::string file_path = TEST_DATA_DIR "/test_data.ohlcv-1d.v1.dbn.zst";
  InFileStream target{file_path};
  std::vector<std::byte> buffer(1024);  // File is less than 1KiB
  const auto read_size = target.ReadSome(buffer.data(), buffer.size());
  ASSERT_GT(read_size, 0);
  ASSERT_TRUE(std::any_of(buffer.cbegin(), buffer.cend(), [](std::byte byte) {
    return std::to_integer<int>(byte) != 0;
  }));
}

TEST(OutFileStreamTests, TestWriteAllCanBeRead) {
  constexpr auto data = "abcdefgh";
  TempFile temp_file{std::filesystem::temp_directory_path() / "out"};
  ASSERT_FALSE(temp_file.Exists());
  {
    OutFileStream target{temp_file.Path()};
    target.WriteAll(reinterpret_cast<const std::byte*>(data), 8);
  }
  ASSERT_TRUE(temp_file.Exists());
  InFileStream input{temp_file.Path()};
  std::vector<char> buf(9);
  input.ReadExact(reinterpret_cast<std::byte*>(buf.data()), 8);
  ASSERT_STREQ(buf.data(), data);
}
}  // namespace databento::tests
