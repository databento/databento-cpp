#pragma once

#include <cstddef>
#include <memory>
#include <ostream>

#include "databento/detail/buffer.hpp"
#include "databento/detail/dbn_fsm.hpp"
#include "databento/detail/zstd_stream.hpp"
#include "databento/enums.hpp"
#include "databento/timeseries.hpp"

namespace databento::detail {
class DbnBufferDecoder {
 public:
  // The instance cannot outlive the lifetime of these references.
  DbnBufferDecoder(VersionUpgradePolicy upgrade_policy,
                   const MetadataCallback& metadata_callback,
                   const RecordCallback& record_callback)
      : metadata_callback_{metadata_callback},
        record_callback_{record_callback},
        zstd_stream_{std::make_unique<Buffer>()},
        zstd_buffer_{static_cast<Buffer*>(zstd_stream_.Input())},
        fsm_{upgrade_policy} {}

  KeepGoing Process(const char* data, std::size_t length);

  std::size_t UnreadBytes() const { return fsm_.UnreadBytes(); }
  friend std::ostream& operator<<(std::ostream& stream, const DbnBufferDecoder& buffer);

 private:
  const MetadataCallback& metadata_callback_;
  const RecordCallback& record_callback_;
  ZstdDecodeStream zstd_stream_;
  Buffer* zstd_buffer_;
  DbnFsm fsm_;
};
}  // namespace databento::detail
