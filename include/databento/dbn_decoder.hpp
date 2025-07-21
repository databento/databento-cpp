#pragma once

#include <cstddef>  // size_t
#include <cstdint>  // uint8_t
#include <memory>   // unique_ptr
#include <string>

#include "databento/dbn.hpp"
#include "databento/detail/buffer.hpp"
#include "databento/enums.hpp"  // Upgrade Policy
#include "databento/file_stream.hpp"
#include "databento/ireadable.hpp"
#include "databento/log.hpp"
#include "databento/record.hpp"  // Record, RecordHeader

namespace databento {
// DBN decoder. Set upgrade_policy to control how DBN version 1 data should be
// handled. Defaults to upgrading DBN versions 1 and 2 to version 3 (the current
// version).
class DbnDecoder {
 public:
  DbnDecoder(ILogReceiver* log_receiver, InFileStream file_stream);
  DbnDecoder(ILogReceiver* log_receiver, std::unique_ptr<IReadable> input);
  DbnDecoder(ILogReceiver* log_receiver, std::unique_ptr<IReadable> input,
             VersionUpgradePolicy upgrade_policy);

  static std::pair<std::uint8_t, std::size_t> DecodeMetadataVersionAndSize(
      const std::byte* buffer, std::size_t size);
  static Metadata DecodeMetadataFields(std::uint8_t version, const std::byte* buffer,
                                       const std::byte* buffer_end);
  // Decodes a record possibly applying upgrading the data according to the
  // given version and upgrade policy. If an upgrade is applied,
  // compat_buffer is modified.
  static Record DecodeRecordCompat(std::uint8_t version,
                                   VersionUpgradePolicy upgrade_policy, bool ts_out,
                                   std::array<std::byte, kMaxRecordLen>* compat_buffer,
                                   Record rec);

  // Should be called exactly once.
  Metadata DecodeMetadata();
  // Lifetime of returned Record is until next call to DecodeRecord. Returns
  // nullptr once the end of the input has been reached.
  const Record* DecodeRecord();

 private:
  static std::string DecodeSymbol(std::size_t symbol_cstr_len,
                                  const std::byte*& buffer);
  static std::vector<std::string> DecodeRepeatedSymbol(std::size_t symbol_cstr_len,
                                                       const std::byte*& buffer,
                                                       const std::byte* buffer_end);
  static std::vector<SymbolMapping> DecodeSymbolMappings(std::size_t symbol_cstr_len,
                                                         const std::byte*& buffer,
                                                         const std::byte* buffer_end);
  static SymbolMapping DecodeSymbolMapping(std::size_t symbol_cstr_len,
                                           const std::byte*& buffer,
                                           const std::byte* buffer_end);
  bool DetectCompression();
  std::size_t FillBuffer();
  RecordHeader* BufferRecordHeader();

  ILogReceiver* log_receiver_;
  std::uint8_t version_{};
  VersionUpgradePolicy upgrade_policy_;
  bool ts_out_{};
  std::unique_ptr<IReadable> input_;
  detail::Buffer buffer_{};
  // Must be 8-byte aligned for records
  alignas(RecordHeader) std::array<std::byte, kMaxRecordLen> compat_buffer_{};
  Record current_record_{nullptr};
};
}  // namespace databento
