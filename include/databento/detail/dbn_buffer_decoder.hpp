#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <ostream>

#include "databento/detail/buffer.hpp"
#include "databento/detail/zstd_stream.hpp"
#include "databento/enums.hpp"
#include "databento/record.hpp"
#include "databento/timeseries.hpp"

namespace databento::detail {
class DbnBufferDecoder {
 public:
  // The instance cannot outlive the lifetime of these references.
  DbnBufferDecoder(VersionUpgradePolicy upgrade_policy,
                   const MetadataCallback& metadata_callback,
                   const RecordCallback& record_callback)
      : upgrade_policy_{upgrade_policy},
        metadata_callback_{metadata_callback},
        record_callback_{record_callback},
        zstd_stream_{std::make_unique<Buffer>()},
        zstd_buffer_{static_cast<Buffer*>(zstd_stream_.Input())} {}

  KeepGoing Process(const char* data, std::size_t length);

  std::size_t UnreadBytes() const { return dbn_buffer_.ReadCapacity(); }
  friend std::ostream& operator<<(std::ostream& stream, const DbnBufferDecoder& buffer);

 private:
  enum class DecoderState : std::uint8_t {
    Init,
    Metadata,
    Records,
  };

  friend std::ostream& operator<<(std::ostream& stream, DecoderState state) {
    switch (state) {
      case DbnBufferDecoder::DecoderState::Init:
        stream << "init";
        break;
      case DbnBufferDecoder::DecoderState::Metadata:
        stream << "metadata";
        break;
      case DbnBufferDecoder::DecoderState::Records:
        stream << "records";
        break;
    }
    return stream;
  }

  const VersionUpgradePolicy upgrade_policy_;
  const MetadataCallback& metadata_callback_;
  const RecordCallback& record_callback_;
  ZstdDecodeStream zstd_stream_;
  Buffer* zstd_buffer_;
  Buffer dbn_buffer_{};
  std::size_t bytes_needed_{};
  alignas(RecordHeader) std::array<std::byte, kMaxRecordLen> compat_buffer_{};
  std::uint8_t input_version_{};
  bool ts_out_{};
  DecoderState state_{DecoderState::Init};
};
}  // namespace databento::detail
