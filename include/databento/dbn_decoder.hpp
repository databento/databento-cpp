#pragma once

#include <cstddef>  // size_t
#include <cstdint>  // uint8_t
#include <memory>   // unique_ptr
#include <string>

#include "databento/dbn.hpp"
#include "databento/detail/file_stream.hpp"
#include "databento/detail/shared_channel.hpp"
#include "databento/enums.hpp"  // Upgrade Policy
#include "databento/ireadable.hpp"
#include "databento/record.hpp"  // Record, RecordHeader

namespace databento {
// DBN decoder. Set upgrade_policy to control how DBN version 1 data should be
// handled. Defaults to upgrading DBNv1 data to version 2 (the current version).
class DbnDecoder {
 public:
  explicit DbnDecoder(detail::SharedChannel channel);
  explicit DbnDecoder(detail::FileStream file_stream);
  explicit DbnDecoder(std::unique_ptr<IReadable> input);
  DbnDecoder(std::unique_ptr<IReadable> input,
             VersionUpgradePolicy upgrade_policy);

  // Decode metadata from the given buffer.
  static Metadata DecodeMetadata(const std::vector<std::uint8_t>& buffer);
  static std::pair<std::uint8_t, std::size_t> DecodeMetadataVersionAndSize(
      const std::uint8_t* buffer, std::size_t size);
  static Metadata DecodeMetadataFields(std::uint8_t version,
                                       const std::vector<std::uint8_t>& buffer);
  // Decodes a record possibly applying upgrading the data according to the
  // given version and upgrade policy. If an upgrade is applied,
  // compat_buffer is modified.
  static Record DecodeRecordCompat(
      std::uint8_t version, VersionUpgradePolicy upgrade_policy,
      std::array<std::uint8_t, kMaxRecordLen>* compat_buffer, Record rec);

  // Should be called exactly once.
  Metadata DecodeMetadata();
  // Lifetime of returned Record is until next call to DecodeRecord. Returns
  // nullptr once the end of the input has been reached.
  const Record* DecodeRecord();

 private:
  static std::string DecodeSymbol(
      std::size_t symbol_cstr_len,
      std::vector<std::uint8_t>::const_iterator& buffer_it);
  static std::vector<std::string> DecodeRepeatedSymbol(
      std::size_t symbol_cstr_len,
      std::vector<std::uint8_t>::const_iterator& buffer_it,
      std::vector<std::uint8_t>::const_iterator buffer_end_it);
  static std::vector<SymbolMapping> DecodeSymbolMappings(
      std::size_t symbol_cstr_len,
      std::vector<std::uint8_t>::const_iterator& buffer_it,
      std::vector<std::uint8_t>::const_iterator buffer_end_it);
  static SymbolMapping DecodeSymbolMapping(
      std::size_t symbol_cstr_len,
      std::vector<std::uint8_t>::const_iterator& buffer_it,
      std::vector<std::uint8_t>::const_iterator buffer_end_it);
  bool DetectCompression();
  std::size_t FillBuffer();
  RecordHeader* BufferRecordHeader();

  std::uint8_t version_{};
  VersionUpgradePolicy upgrade_policy_;
  std::unique_ptr<IReadable> input_;
  std::vector<std::uint8_t> read_buffer_;
  std::size_t buffer_idx_{};
  // Must be 8-byte aligned for records
  alignas(
      RecordHeader) std::array<std::uint8_t, kMaxRecordLen> compat_buffer_{};
  Record current_record_{nullptr};
};
}  // namespace databento
