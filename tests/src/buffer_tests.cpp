#include <gtest/gtest.h>

#include <array>
#include <string_view>

#include "databento/detail/buffer.hpp"

using namespace std::string_view_literals;

namespace databento::detail::tests {
TEST(BufferTests, TestWriteAllPastCapacity) {
  Buffer target{10};
  target.WriteBegin() += 4;
  target.ReadBegin() += 2;
  ASSERT_EQ(target.WriteCapacity(), 6);
  ASSERT_EQ(target.ReadCapacity(), 2);
  ASSERT_EQ(target.Capacity(), 10);

  target.WriteAll("TestWriteAllPastCapacity", 24);
  ASSERT_EQ(target.WriteCapacity(), 0);
  ASSERT_EQ(target.ReadCapacity(), 26);
  ASSERT_EQ(target.Capacity(), 26);
}

TEST(BufferTests, TestWriteAllShift) {
  Buffer target{20};
  target.WriteAll("TestWriteAllShift", 17);
  target.ReadBegin() += 4;
  ASSERT_EQ(target.WriteCapacity(), 3);
  ASSERT_EQ(target.ReadCapacity(), 13);
  ASSERT_EQ(target.Capacity(), 20);

  target.WriteAll("Test", 4);
  ASSERT_EQ(target.WriteCapacity(), 3);
  ASSERT_EQ(target.ReadCapacity(), 17);
  ASSERT_EQ(target.Capacity(), 20);
}

TEST(BufferTests, TestWriteRead) {
  Buffer target{10};
  target.WriteBegin() += 5;
  target.ReadBegin() += 5;
  const auto write_len = target.Write("BufferTests", 11);
  ASSERT_EQ(write_len, 10);
  std::array<std::byte, 10> read_buf{};
  target.ReadExact(read_buf.data(), read_buf.size());
  std::string_view res{reinterpret_cast<const char*>(read_buf.data()),
                       read_buf.size()};
  ASSERT_EQ(res, "BufferTest"sv);
}

TEST(BufferTests, TestReserve) {
  Buffer target{120};
  ASSERT_EQ(target.WriteCapacity(), 120);
  ASSERT_EQ(target.ReadCapacity(), 0);
  ASSERT_EQ(target.Capacity(), 120);
  target.WriteAll("TestReserve", 11);
  target.ReadBegin() += 4;
}

}  // namespace databento::detail::tests
