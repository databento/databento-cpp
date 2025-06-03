#pragma once

#include <cstddef>     // byte, size_t
#include <filesystem>  // path
#include <fstream>     // ifstream, ofstream

#include "databento/ireadable.hpp"
#include "databento/iwritable.hpp"

namespace databento {
class InFileStream : public IReadable {
 public:
  explicit InFileStream(const std::filesystem::path& file_path);

  // Read exactly `length` bytes into `buffer`.
  void ReadExact(std::byte* buffer, std::size_t length) override;
  // Read at most `length` bytes. Returns the number of bytes read. Will only
  // return 0 if the end of the stream is reached.
  std::size_t ReadSome(std::byte* buffer, std::size_t max_length) override;

 private:
  std::ifstream stream_;
};

class OutFileStream : public IWritable {
 public:
  explicit OutFileStream(const std::filesystem::path& file_path);

  void WriteAll(const std::byte* buffer, std::size_t length) override;

 private:
  std::ofstream stream_;
};
}  // namespace databento
