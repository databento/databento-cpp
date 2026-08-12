#include "databento/detail/dbn_buffer_decoder.hpp"

#include "detail/stream_op_helper.hpp"

using databento::detail::DbnBufferDecoder;

databento::KeepGoing DbnBufferDecoder::Process(const char* data, std::size_t length) {
  zstd_buffer_->WriteAll(data, length);
  while (true) {
    std::size_t space_length{};
    auto* space = fsm_.Space(&space_length);
    const auto read_size = zstd_stream_.ReadSome(space, space_length);
    fsm_.Fill(read_size);
    if (read_size == 0) {
      return KeepGoing::Continue;
    }
    bool read_more = false;
    while (!read_more) {
      switch (fsm_.Process()) {
        case DbnFsm::Status::ReadMore: {
          read_more = true;
          break;
        }
        case DbnFsm::Status::Metadata: {
          if (metadata_callback_) {
            metadata_callback_(fsm_.TakeMetadata());
          }
          break;
        }
        case DbnFsm::Status::Record: {
          if (record_callback_(fsm_.LastRecord()) == KeepGoing::Stop) {
            return KeepGoing::Stop;
          }
          break;
        }
      }
    }
  }
}

namespace databento::detail {
std::ostream& operator<<(std::ostream& stream, const DbnBufferDecoder& buffer) {
  return StreamOpBuilder{stream}
      .SetSpacer("\n    ")
      .SetTypeName("DbnBufferDecoder")
      .Build()
      .AddField("unread_bytes", buffer.UnreadBytes())
      .Finish();
}
}  // namespace databento::detail
