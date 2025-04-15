#pragma once

#include <cstddef>
#include <vector>

#include "databento/ireadable.hpp"
#include "databento/iwritable.hpp"

namespace databento::tests::mock {
class MockIo : public databento::IWritable, public databento::IReadable {
 public:
  void WriteAll(const std::byte* buffer, std::size_t length);

  void ReadExact(std::byte* buffer, std::size_t length);

  std::size_t ReadSome(std::byte* buffer, std::size_t max_length);

  const std::vector<std::byte>& GetContents() const { return contents_; }

 private:
  std::vector<std::byte> contents_;
  std::ptrdiff_t read_idx_{0};
};
}  // namespace databento::tests::mock
