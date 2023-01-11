#pragma once

#include <zstd.h>

#include <cstddef>  // size_t
#include <cstdint>  // uint8_t
#include <string>
#include <type_traits>  // is_nothrow_move_constructible
#include <vector>

#include "databento/dbn.hpp"
#include "databento/detail/file_stream.hpp"
#include "databento/detail/shared_channel.hpp"
#include "databento/record.hpp"

namespace databento {
// DBN parser. Use either the DbnChannelParser or DbnFileParser specialization.
template <typename Input>
class DbnParser {
 public:
  explicit DbnParser(Input input) : input_{std::move(input)} {}
  DbnParser(const DbnParser&) = delete;
  DbnParser& operator=(const DbnParser&) = delete;
  DbnParser(DbnParser&&) noexcept(
      // in some STL implementations, ifstream is not noexcept move
      // constructible
      std::is_nothrow_move_constructible<Input>::value) = default;
  DbnParser& operator=(DbnParser&&) noexcept(
      std::is_nothrow_move_constructible<Input>::value) = default;
  ~DbnParser();

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
extern template class DbnParser<detail::SharedChannel>;
extern template class DbnParser<detail::FileStream>;
// DBN parser that reads from a channel.
using DbnChannelParser = DbnParser<detail::SharedChannel>;
// DBN parser that reads from a file.
using DbnFileParser = DbnParser<detail::FileStream>;
}  // namespace databento
