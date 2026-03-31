#include "databento/dbn_store.hpp"

#include <memory>   // unique_ptr
#include <utility>  // move

#include "databento/file_stream.hpp"
#include "databento/record.hpp"

using databento::DbnStore;

DbnStore::DbnStore(const std::filesystem::path& file_path)
    : decoder_{ILogReceiver::Default(), InFileStream{file_path}} {}

DbnStore::DbnStore(ILogReceiver* log_receiver, const std::filesystem::path& file_path,
                   VersionUpgradePolicy upgrade_policy)
    : decoder_{log_receiver, std::make_unique<InFileStream>(file_path),
               upgrade_policy} {}

DbnStore::DbnStore(ILogReceiver* log_receiver, std::unique_ptr<IReadable> input,
                   VersionUpgradePolicy upgrade_policy)
    : decoder_{log_receiver, std::move(input), upgrade_policy} {}

void DbnStore::Replay(const MetadataCallback& metadata_callback,
                      const RecordCallback& record_callback) {
  auto metadata = decoder_.DecodeMetadata();
  if (metadata_callback) {
    metadata_callback(std::move(metadata));
  }
  const databento::Record* record;
  while ((record = decoder_.DecodeRecord()) != nullptr) {
    if (record_callback(*record) == KeepGoing::Stop) {
      break;
    }
  }
}

void DbnStore::Replay(const RecordCallback& record_callback) {
  Replay({}, record_callback);
}

const databento::Metadata& DbnStore::GetMetadata() {
  MaybeDecodeMetadata();
  return metadata_;
}

const databento::Record* DbnStore::NextRecord() {
  MaybeDecodeMetadata();
  return decoder_.DecodeRecord();
}

void DbnStore::MaybeDecodeMetadata() {
  if (!has_decoded_metadata_) {
    metadata_ = decoder_.DecodeMetadata();
    has_decoded_metadata_ = true;
  }
}
