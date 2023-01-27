#pragma once

#include <cstddef>  // size_t
#include <cstdint>  // uint8_t
#include <memory>   // unique_ptr
#include <string>

#include "databento/dbn.hpp"
#include "databento/detail/file_stream.hpp"
#include "databento/detail/shared_channel.hpp"
#include "databento/ireadable.hpp"
#include "databento/record.hpp"

namespace databento {
// DBN decoder. Use either the DbnChannelDecoder or DbnFileDecoder
// specialization.
class DbnDecoder {
 public:
  explicit DbnDecoder(detail::SharedChannel channel);
  explicit DbnDecoder(detail::FileStream file_stream);
  explicit DbnDecoder(std::unique_ptr<IReadable> input);

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
  bool DetectCompression();
  std::size_t FillBuffer();
  RecordHeader* BufferRecordHeader();

  std::unique_ptr<IReadable> input_;
  std::vector<std::uint8_t> buffer_;
  std::size_t buffer_idx_{};
};
}  // namespace databento
