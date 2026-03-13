#pragma once

#include <filesystem>  // path
#include <memory>      // unique_ptr

#include "databento/dbn.hpp"          // DecodeMetadata
#include "databento/dbn_decoder.hpp"  // DbnDecoder
#include "databento/enums.hpp"        // VersionUpgradePolicy
#include "databento/ireadable.hpp"
#include "databento/log.hpp"
#include "databento/record.hpp"
#include "databento/timeseries.hpp"  // MetadataCallback, RecordCallback

namespace databento {
// A reader for DBN data from files or streams. This class provides both a callback API
// similar to `LiveThreaded` for live data as well as a blocking API similar to that of
// `LiveBlocking`. Only one API should be used on a given instance.
class DbnStore {
 public:
  explicit DbnStore(const std::filesystem::path& file_path);
  DbnStore(ILogReceiver* log_receiver, const std::filesystem::path& file_path,
           VersionUpgradePolicy upgrade_policy);
  DbnStore(ILogReceiver* log_receiver, std::unique_ptr<IReadable> input,
           VersionUpgradePolicy upgrade_policy);

  // Callback API: calling Replay consumes the input.
  void Replay(const MetadataCallback& metadata_callback,
              const RecordCallback& record_callback);
  void Replay(const RecordCallback& record_callback);

  // Blocking API
  const databento::Metadata& GetMetadata();
  // Returns the next record or `nullptr` if there are no remaining records.
  const Record* NextRecord();

 private:
  void MaybeDecodeMetadata();

  DbnDecoder decoder_;
  databento::Metadata metadata_{};
  bool has_decoded_metadata_{false};
};
}  // namespace databento
