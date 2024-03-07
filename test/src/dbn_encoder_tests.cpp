#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>  // ifstream
#include <ios>      // streamsize, ios::binary, ios::ate
#include <memory>

#include "databento/compat.hpp"
#include "databento/constants.hpp"
#include "databento/dbn.hpp"
#include "databento/dbn_decoder.hpp"
#include "databento/dbn_encoder.hpp"
#include "databento/detail/file_stream.hpp"
#include "databento/detail/scoped_thread.hpp"
#include "databento/detail/shared_channel.hpp"
#include "databento/enums.hpp"
#include "databento/exceptions.hpp"
#include "databento/ireadable.hpp"
#include "databento/record.hpp"

namespace databento {
namespace test {
class DbnEncoderTests : public testing::Test {
 public:
  detail::SharedChannel channel_;
  std::unique_ptr<DbnDecoder> file_target_;
  std::unique_ptr<DbnDecoder> channel_target_;
  detail::ScopedThread write_thread_;

  void ReadFromFile(const std::string& schema_str, const std::string& extension,
                    std::uint8_t version) {
    ReadFromFile(schema_str, extension, version, VersionUpgradePolicy::AsIs);
  }

  void ReadFromFile(const std::string& schema_str, const std::string& extension,
                    std::uint8_t version, VersionUpgradePolicy upgrade_policy) {
    const char* version_str = version == 1 ? ".v1" : "";
    const std::string file_path = TEST_BUILD_DIR "/data/test_data." +
                                  schema_str + version_str + extension;
    // Channel setup
    write_thread_ = detail::ScopedThread{[this, file_path] {
      std::ifstream input_file{file_path, std::ios::binary | std::ios::ate};
      ASSERT_TRUE(input_file.good());
      const auto size = static_cast<std::size_t>(input_file.tellg());
      input_file.seekg(0, std::ios::beg);
      std::vector<char> buffer(size);
      input_file.read(buffer.data(), static_cast<std::streamsize>(size));
      ASSERT_EQ(input_file.gcount(), size);
      channel_.Write(reinterpret_cast<const std::uint8_t*>(buffer.data()),
                     size);
      channel_.Finish();
    }};
    channel_target_.reset(new DbnDecoder{
      std::unique_ptr<IReadable>{new detail::SharedChannel{channel_}},
      upgrade_policy});
    // File setup
    file_target_.reset(new DbnDecoder{
      std::unique_ptr<IReadable>{new detail::FileStream{file_path}},
      upgrade_policy});
  }
};


struct FakeWritable : IWritable {
  std::vector<std::uint8_t> written_bytes;
  void Write(const std::uint8_t* buffer, std::size_t length) override {
    written_bytes.insert(written_bytes.end(), buffer, buffer + length);
  }
};

struct FakeReadable : IReadable {
  std::vector<std::uint8_t> bytes;
  void ReadExact(std::uint8_t* buffer, std::size_t length) override {
    std::memmove(buffer, bytes.data(), length);
    bytes.erase(bytes.begin(), bytes.begin()+length);
  }

  std::size_t ReadSome(std::uint8_t* buffer, std::size_t max_length) override {
    std::size_t  len = std::min(max_length, bytes.size());
    std::memmove(buffer, bytes.data(), len);
    bytes.erase(bytes.begin(), bytes.begin()+len);
    return len;
  }
};

TEST_F(DbnEncoderTests, TestDecodeDefinitionUpgrade) {
  ReadFromFile("definition", ".dbn", 1, VersionUpgradePolicy::Upgrade);

  FakeWritable fake_writable;

  const Metadata ch_metadata = channel_target_->DecodeMetadata();
  DbnEncoder::EncodeMetadata(ch_metadata, fake_writable);

  auto fake_readable = std::make_unique<FakeReadable>();
  fake_readable->bytes = fake_writable.written_bytes;
  DbnDecoder decoder(std::move(fake_readable), databento::VersionUpgradePolicy::AsIs);

  Metadata decoded = decoder.DecodeMetadata();

  EXPECT_EQ(ch_metadata, decoded);
}

}  // namespace test
}  // namespace databento
