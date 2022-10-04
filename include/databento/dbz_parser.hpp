#pragma once

#include <zstd.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "databento/record.hpp"
#include "dbz.hpp"
#include "parse_stream.hpp"

namespace databento {
// Thread-safe DBZ parser
class DbzParser {
  ParseStream stream_;
  std::uint8_t rtype_;
  std::vector<std::uint8_t> in_buffer_;
  std::vector<std::uint8_t> out_buffer_;
  // ZSTD streaming decompression suggests the size of the next read
  std::size_t read_suggestion_;
  ZSTD_DStream* z_dstream_;
  ZSTD_inBuffer z_in_buffer_;
  ZSTD_outBuffer z_out_buffer_;

 public:
  DbzParser() = default;
  DbzParser(const DbzParser&) = delete;
  DbzParser& operator=(const DbzParser&) = delete;
  DbzParser(DbzParser&&) = delete;
  DbzParser& operator=(DbzParser&&) = delete;
  ~DbzParser();

  // Write to the stream to be parsed.
  void PassBytes(const std::uint8_t* data, std::size_t length);
  void EndInput();
  // Should only be called once
  Metadata ParseMetadata();
  // Lifetime of returned Record is until next call to ParseRecord.
  Record ParseRecord();

 private:
  template <typename T>
  static T FromLittleEndianBytes(const std::uint8_t* bytes);
  template <typename T>
  static T Consume(std::vector<std::uint8_t>::const_iterator& byte_it);
  static const char* Consume(std::vector<std::uint8_t>::const_iterator& byte_it,
                             const std::ptrdiff_t num_bytes);

  static std::vector<std::string> ParseRepeatedCstr(
      std::vector<std::uint8_t>::const_iterator& buffer_it,
      std::vector<std::uint8_t>::const_iterator buffer_end_it);
};

template <>
std::uint64_t DbzParser::FromLittleEndianBytes(const std::uint8_t* bytes);
template <>
std::uint32_t DbzParser::FromLittleEndianBytes(const std::uint8_t* bytes);
template <>
std::uint16_t DbzParser::FromLittleEndianBytes(const std::uint8_t* bytes);
}  // namespace databento
