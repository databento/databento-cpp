#pragma once

#include <filesystem>  // path

#include "databento/dbn.hpp"          // DecodeMetadata
#include "databento/dbn_decoder.hpp"  // DbnDecoder
#include "databento/enums.hpp"        // VersionUpgradePolicy
#include "databento/log.hpp"
#include "databento/record.hpp"
#include "databento/timeseries.hpp"  // MetadataCallback, RecordCallback

namespace databento {
// A reader for DBN files. This class provides both a callback API similar to
// TimeseriesGetRange in historical data and LiveThreaded for live data as well
// as a blocking API similar to that of LiveBlocking. Only one API should be
// used on a given instance.
class DbnFileStore {
 public:
  explicit DbnFileStore(const std::filesystem::path& file_path);
  DbnFileStore(ILogReceiver* log_receiver, const std::filesystem::path& file_path,
               VersionUpgradePolicy upgrade_policy);

  // Callback API: calling Replay consumes the input.
  void Replay(const MetadataCallback& metadata_callback,
              const RecordCallback& record_callback);
  void Replay(const RecordCallback& record_callback);

  // Blocking API
  const Metadata& GetMetadata();
  // Returns the next record or `nullptr` if there are no remaining records.
  const Record* NextRecord();

 private:
  void MaybeDecodeMetadata();

  DbnDecoder decoder_;
  Metadata metadata_{};
  bool has_decoded_metadata_{false};
};
}  // namespace databento
