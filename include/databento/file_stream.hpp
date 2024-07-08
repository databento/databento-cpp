#pragma once

#include <cstddef>  // size_t
#include <cstdint>  // uint8_t
#include <fstream>  // ifstream, ofstream
#include <string>

#include "databento/ireadable.hpp"
#include "databento/iwritable.hpp"

namespace databento {
class InFileStream : public IReadable {
 public:
  explicit InFileStream(const std::string& file_path);

  // Read exactly `length` bytes into `buffer`.
  void ReadExact(std::uint8_t* buffer, std::size_t length) override;
  // Read at most `length` bytes. Returns the number of bytes read. Will only
  // return 0 if the end of the stream is reached.
  std::size_t ReadSome(std::uint8_t* buffer, std::size_t max_length) override;

 private:
  std::ifstream stream_;
};

class OutFileStream : public IWritable {
 public:
  explicit OutFileStream(const std::string& file_path);

  void WriteAll(const std::uint8_t* buffer, std::size_t length) override;

 private:
  std::ofstream stream_;
};
}  // namespace databento
