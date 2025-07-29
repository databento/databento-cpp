#include "databento/detail/dbn_buffer_decoder.hpp"

#include "databento/dbn_decoder.hpp"
#include "databento/timeseries.hpp"
#include "dbn_constants.hpp"
#include "stream_op_helper.hpp"

using databento::detail::DbnBufferDecoder;

databento::KeepGoing DbnBufferDecoder::Process(const char* data, std::size_t length) {
  zstd_buffer_->WriteAll(data, length);
  while (true) {
    const auto read_size =
        zstd_stream_.ReadSome(dbn_buffer_.WriteBegin(), dbn_buffer_.WriteCapacity());
    dbn_buffer_.Fill(read_size);
    if (read_size == 0) {
      return KeepGoing::Continue;
    }
    switch (state_) {
      case DecoderState::Init: {
        if (dbn_buffer_.ReadCapacity() < kMetadataPreludeSize) {
          break;
        }
        std::tie(input_version_, bytes_needed_) =
            DbnDecoder::DecodeMetadataVersionAndSize(dbn_buffer_.ReadBegin(),
                                                     dbn_buffer_.ReadCapacity());
        dbn_buffer_.Consume(kMetadataPreludeSize);
        dbn_buffer_.Reserve(bytes_needed_);
        state_ = DecoderState::Metadata;
        [[fallthrough]];
      }
      case DecoderState::Metadata: {
        if (dbn_buffer_.ReadCapacity() < bytes_needed_) {
          break;
        }
        auto metadata = DbnDecoder::DecodeMetadataFields(
            input_version_, dbn_buffer_.ReadBegin(), dbn_buffer_.ReadEnd());
        dbn_buffer_.Consume(bytes_needed_);
        // Metadata may leave buffer misaligned. Shift records to ensure 8-byte
        // alignment
        dbn_buffer_.Shift();
        ts_out_ = metadata.ts_out;
        metadata.Upgrade(upgrade_policy_);
        if (metadata_callback_) {
          metadata_callback_(std::move(metadata));
        }
        state_ = DecoderState::Records;
        [[fallthrough]];
      }
      case DecoderState::Records: {
        while (dbn_buffer_.ReadCapacity() > 0) {
          auto record =
              Record{reinterpret_cast<RecordHeader*>(dbn_buffer_.ReadBegin())};
          bytes_needed_ = record.Size();
          if (dbn_buffer_.ReadCapacity() < bytes_needed_) {
            break;
          }
          record = DbnDecoder::DecodeRecordCompat(input_version_, upgrade_policy_,
                                                  ts_out_, &compat_buffer_, record);
          if (record_callback_(record) == KeepGoing::Stop) {
            return KeepGoing::Stop;
          }
          dbn_buffer_.Consume(bytes_needed_);
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
      .AddField("dbn_buffer_", buffer.dbn_buffer_)
      .AddField("bytes_needed_", buffer.bytes_needed_)
      .AddField("input_version_", buffer.input_version_)
      .AddField("ts_out_", buffer.ts_out_)
      .AddField("state_", buffer.state_)
      .Finish();
}
}  // namespace databento::detail
