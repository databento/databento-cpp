#include "databento/dbn_decoder.hpp"

#include <array>
#include <cstring>  // strncmp
#include <string>

#include "databento/detail/buffer.hpp"
#include "databento/detail/zstd_stream.hpp"
#include "databento/exceptions.hpp"
#include "dbn_constants.hpp"

using databento::DbnDecoder;

DbnDecoder::DbnDecoder(ILogReceiver* log_receiver, InFileStream file_stream)
    : DbnDecoder(log_receiver, std::make_unique<InFileStream>(std::move(file_stream))) {
}

DbnDecoder::DbnDecoder(ILogReceiver* log_receiver, std::unique_ptr<IReadable> input)
    : DbnDecoder(log_receiver, std::move(input), VersionUpgradePolicy::UpgradeToV3) {}

DbnDecoder::DbnDecoder(ILogReceiver* log_receiver, std::unique_ptr<IReadable> input,
                       VersionUpgradePolicy upgrade_policy)
    : log_receiver_{log_receiver}, input_{std::move(input)}, fsm_{upgrade_policy} {
  DetectCompression();
}

databento::Metadata DbnDecoder::DecodeMetadata() {
  while (true) {
    switch (fsm_.Process()) {
      case detail::DbnFsm::Status::Metadata: {
        return fsm_.TakeMetadata();
      }
      case detail::DbnFsm::Status::Record: {
        throw DbnResponseError{"Found a record before the metadata"};
      }
      case detail::DbnFsm::Status::ReadMore: {
        if (FillBuffer() == 0) {
          throw DbnResponseError{"Unexpected end of input before the metadata"};
        }
      }
    }
  }
}

const databento::Record* DbnDecoder::DecodeRecord() {
  while (true) {
    switch (fsm_.Process()) {
      case detail::DbnFsm::Status::Record: {
        return &fsm_.LastRecord();
      }
      case detail::DbnFsm::Status::Metadata: {
        break;
      }
      case detail::DbnFsm::Status::ReadMore: {
        if (FillBuffer() == 0) {
          if (fsm_.UnreadBytes() > 0) {
            log_receiver_->Receive(LogLevel::Warning,
                                   "Unexpected partial record remaining in stream: " +
                                       std::to_string(fsm_.UnreadBytes()) + " bytes");
          }
          return nullptr;
        }
      }
    }
  }
}

std::size_t DbnDecoder::FillBuffer() {
  std::size_t length{};
  auto* space = fsm_.Space(&length);
  const auto fill_size = input_->ReadSome(space, length);
  fsm_.Fill(fill_size);
  return fill_size;
}

void DbnDecoder::DetectCompression() {
  std::array<std::byte, kMagicSize> magic{};
  input_->ReadExact(magic.data(), magic.size());
  if (std::strncmp(reinterpret_cast<const char*>(magic.data()), kDbnPrefix, 3) == 0) {
    fsm_.WriteAll(magic.data(), magic.size());
    return;
  }
  const auto first_word = *reinterpret_cast<const std::uint32_t*>(magic.data());
  // Zstandard skippable frames begin with 0x184D2A5? where the last 8 bits
  // can be set to any value
  constexpr auto kZstdSkippableFrame = 0x184D2A50;
  if (first_word != kZstdMagicNumber) {
    if ((first_word & kZstdSkippableFrame) == kZstdSkippableFrame) {
      throw DbnResponseError{
          "Legacy DBZ encoding is not supported. Please use the dbn CLI tool "
          "to convert it to DBN."};
    }
    throw DbnResponseError{
        "Couldn't detect input type. It doesn't appear to be Zstd or DBN."};
  }
  detail::Buffer zstd_magic{kMagicSize};
  zstd_magic.WriteAll(magic.data(), magic.size());
  input_ = std::make_unique<detail::ZstdDecodeStream>(std::move(input_), zstd_magic);
  input_->ReadExact(magic.data(), magic.size());
  if (std::strncmp(reinterpret_cast<const char*>(magic.data()), kDbnPrefix, 3) != 0) {
    throw DbnResponseError{"Found Zstd input, but not DBN prefix"};
  }
  fsm_.WriteAll(magic.data(), magic.size());
}
