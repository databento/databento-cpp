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

  // Decode metadata from the given buffer.
  static Metadata DecodeMetadata(const std::vector<std::uint8_t>& buffer);
  static std::pair<std::uint8_t, std::size_t> DecodeMetadataVersionAndSize(
      const std::uint8_t* buffer, std::size_t size);
  static Metadata DecodeMetadataFields(std::uint8_t version,
                                       const std::vector<std::uint8_t>& buffer);

  // Should only be called once
  Metadata DecodeMetadata();
  // Lifetime of returned Record is until next call to ParseRecord.
  Record DecodeRecord();

 private:
  static std::string DecodeSymbol(
      std::vector<std::uint8_t>::const_iterator& buffer_it);
  static std::vector<std::string> DecodeRepeatedSymbol(
      std::vector<std::uint8_t>::const_iterator& buffer_it,
      std::vector<std::uint8_t>::const_iterator buffer_end_it);
  static std::vector<SymbolMapping> DecodeSymbolMappings(
      std::vector<std::uint8_t>::const_iterator& buffer_it,
      std::vector<std::uint8_t>::const_iterator buffer_end_it);
  static SymbolMapping DecodeSymbolMapping(
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
