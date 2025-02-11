#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "databento/ireadable.hpp"
#include "databento/iwritable.hpp"

namespace databento::tests::mock {
class MockIo : public databento::IWritable, public databento::IReadable {
 public:
  void WriteAll(const std::uint8_t* buffer, std::size_t length);

  void ReadExact(std::uint8_t* buffer, std::size_t length);

  std::size_t ReadSome(std::uint8_t* buffer, std::size_t max_length);

  const std::vector<std::uint8_t>& GetContents() const { return contents_; }

 private:
  std::vector<std::uint8_t> contents_;
  std::ptrdiff_t read_idx_{0};
};
}  // namespace databento::tests::mock
