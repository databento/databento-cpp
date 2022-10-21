#pragma once

#include <zstd.h>

#include <cstddef>  // size_t
#include <cstdint>  // uint8_t
#include <string>
#include <type_traits>  // is_nothrow_move_constructible
#include <vector>

#include "databento/dbz.hpp"
#include "databento/detail/file_stream.hpp"
#include "databento/detail/shared_channel.hpp"
#include "databento/record.hpp"

namespace databento {
// DBZ parser. Use either the DbzChannelParser or DbzFileParser specialization.
template <typename Input>
class DbzParser {
 public:
  explicit DbzParser(Input input) : input_{std::move(input)} {}
  DbzParser(const DbzParser&) = delete;
  DbzParser& operator=(const DbzParser&) = delete;
  DbzParser(DbzParser&&) noexcept(
      // in some STL implementations, ifstream is not noexcept move
      // constructible
      std::is_nothrow_move_constructible<Input>::value) = default;
  DbzParser& operator=(DbzParser&&) noexcept(
      std::is_nothrow_move_constructible<Input>::value) = default;
  ~DbzParser();

  // Should only be called once
  Metadata ParseMetadata();
  // Lifetime of returned Record is until next call to ParseRecord.
  Record ParseRecord();

 private:
  static std::string ParseSymbol(
      std::vector<std::uint8_t>::const_iterator& buffer_it);
  static std::vector<std::string> ParseRepeatedSymbol(
      std::vector<std::uint8_t>::const_iterator& buffer_it,
      std::vector<std::uint8_t>::const_iterator buffer_end_it);
  static std::vector<SymbolMapping> ParseSymbolMappings(
      std::vector<std::uint8_t>::const_iterator& buffer_it,
      std::vector<std::uint8_t>::const_iterator buffer_end_it);
  static SymbolMapping ParseSymbolMapping(
      std::vector<std::uint8_t>::const_iterator& buffer_it,
      std::vector<std::uint8_t>::const_iterator buffer_end_it);

  Input input_;
  std::uint8_t rtype_{};
  std::vector<std::uint8_t> in_buffer_;
  std::vector<std::uint8_t> out_buffer_;
  // ZSTD streaming decompression suggests the size of the next read
  std::size_t read_suggestion_{};
  ZSTD_DStream* z_dstream_{};
  ZSTD_inBuffer z_in_buffer_{};
  ZSTD_outBuffer z_out_buffer_{};
};

// forward declare template instantiation
extern template class DbzParser<detail::SharedChannel>;
extern template class DbzParser<detail::FileStream>;
// DBZ parser that reads from a channel.
using DbzChannelParser = DbzParser<detail::SharedChannel>;
// DBZ parser that reads from a file.
using DbzFileParser = DbzParser<detail::FileStream>;
}  // namespace databento
