#include <gtest/gtest.h>

#include <array>
#include <string_view>

#include "databento/detail/buffer.hpp"

using namespace std::string_view_literals;

namespace databento::detail::tests {
TEST(BufferTests, TestWriteAllPastCapacity) {
  Buffer target{10};
  target.Fill(4);
  target.Consume(2);
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
  target.Consume(4);
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
  target.Fill(5);
  target.Consume(5);
  const auto write_len = target.Write("BufferTests", 11);
  ASSERT_EQ(write_len, 10);
  std::array<std::byte, 10> read_buf{};
  target.ReadExact(read_buf.data(), read_buf.size());
  std::string_view res{reinterpret_cast<const char*>(read_buf.data()), read_buf.size()};
  ASSERT_EQ(res, "BufferTest"sv);
}

TEST(BufferTests, TestReserve) {
  Buffer target{120};
  ASSERT_EQ(target.WriteCapacity(), 120);
  ASSERT_EQ(target.ReadCapacity(), 0);
  ASSERT_EQ(target.Capacity(), 120);
  target.WriteAll("TestReserve", 11);
  target.Consume(4);
}

TEST(BufferTests, TestConsumeDoesNotShift) {
  Buffer target{16};
  target.Fill(12);
  target.Consume(10);
  ASSERT_EQ(target.ReadCapacity(), 2);
  ASSERT_EQ(target.WriteCapacity(), 4);
}

TEST(BufferTests, TestShiftForSpace) {
  Buffer target{120};
  target.Fill(40);
  target.Consume(20);
  ASSERT_EQ(target.WriteCapacity(), 80);
  ASSERT_EQ(target.ReadCapacity(), 20);
  // Writable space is sufficient: no shift
  target.ShiftForSpace(50);
  ASSERT_EQ(target.WriteCapacity(), 80);
  ASSERT_EQ(target.ReadCapacity(), 20);
  // Writable space is insufficient: reclaim the consumed prefix
  target.ShiftForSpace(100);
  ASSERT_EQ(target.WriteCapacity(), 100);
  ASSERT_EQ(target.ReadCapacity(), 20);
  // Nothing left to reclaim; shift is a no-op
  target.ShiftForSpace(1000);
  ASSERT_EQ(target.WriteCapacity(), 100);
  ASSERT_EQ(target.ReadCapacity(), 20);
}

TEST(BufferTests, TestShiftForSpaceNoopWhenUnconsumed) {
  Buffer target{16};
  target.Fill(4);
  target.ShiftForSpace(1000);
  ASSERT_EQ(target.WriteCapacity(), 12);
  ASSERT_EQ(target.ReadCapacity(), 4);
}
}  // namespace databento::detail::tests
