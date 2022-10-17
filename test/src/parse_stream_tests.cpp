#include <gtest/gtest.h>

#include <array>
#include <chrono>
#include <cstdint>
#include <string>
#include <thread>
#include <vector>

#include "databento/exceptions.hpp"
#include "parse_stream.hpp"

namespace databento {
namespace test {
class ParseStreamTests : public testing::Test {
 protected:
  std::thread write_thread_;
  ParseStream target_;

 public:
  void TearDown() override {
    if (write_thread_.joinable()) {
      write_thread_.join();
    }
  }

  void Write(const std::vector<std::string>& inputs) {
    for (const auto& input : inputs) {
      target_.Write(reinterpret_cast<const std::uint8_t*>(input.data()),
                    input.size());
      std::this_thread::sleep_for(std::chrono::milliseconds{10});
    }
    target_.Finish();
  }
};

TEST_F(ParseStreamTests, TestReadExact) {
  write_thread_ = std::thread([this] {
    this->Write({"parse", "stream", "tests", "end"});
  });
  std::array<std::uint8_t, 16> buffer{};
  target_.ReadExact(buffer.data(), 3);
  EXPECT_STREQ(reinterpret_cast<const char*>(buffer.data()), "par");
  target_.ReadExact(buffer.data(), 8);
  EXPECT_STREQ(reinterpret_cast<const char*>(buffer.data()), "sestream");
  target_.ReadExact(buffer.data(), 8);
  EXPECT_STREQ(reinterpret_cast<const char*>(buffer.data()), "testsend");
  ASSERT_THROW(target_.ReadExact(buffer.data(), 1), DbzResponseError);
}

TEST_F(ParseStreamTests, TestReadExactAfterFinished) {
  // write on same thread, so all reading happens after writing
  this->Write({"parse", "exact"});
  std::array<std::uint8_t, 16> buffer{};
  target_.ReadExact(buffer.data(), 7);
  EXPECT_STREQ(reinterpret_cast<const char*>(buffer.data()), "parseex");
  // reset buffer
  buffer = {};
  target_.ReadExact(buffer.data(), 3);
  EXPECT_STREQ(reinterpret_cast<const char*>(buffer.data()), "act");
}

TEST_F(ParseStreamTests, TestInterleavedReadsAndWrites) {
  std::array<std::uint8_t, 16> buffer{};
  target_.Write(reinterpret_cast<const std::uint8_t*>("hello"), 5);
  ASSERT_EQ(target_.ReadSome(buffer.data(), buffer.size()), 5);
  EXPECT_STREQ(reinterpret_cast<const char*>(buffer.data()), "hello");
  buffer = {};
  target_.Write(reinterpret_cast<const std::uint8_t*>("longer message"), 14);
  target_.Finish();
  target_.ReadSome(buffer.data(), 6);
  target_.ReadSome(&buffer[6], 1);
  target_.ReadSome(&buffer[7], 7);
  EXPECT_STREQ(reinterpret_cast<const char*>(buffer.data()), "longer message");
}

TEST_F(ParseStreamTests, TestReadSome) {
  write_thread_ = std::thread([this] {
    this->Write({"parse", "stream", "tests", "some", "last"});
  });
  std::array<std::uint8_t, 16> buffer{};
  std::string res;
  // -1 to keep last null byte
  while (res.size() < 23) {
    auto read_size = target_.ReadSome(buffer.data(), buffer.size());
    res.append(reinterpret_cast<const char*>(buffer.data()), read_size);
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    buffer = {};
  }

  ASSERT_EQ(res, "parsestreamtestssomelast");
}
}  // namespace test
}  // namespace databento
