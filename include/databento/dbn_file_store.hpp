#pragma once

#include <string>

#include "databento/dbn_decoder.hpp"  // DbnDecoder
#include "databento/enums.hpp"        // VersionUpgradePolicy
#include "databento/timeseries.hpp"   // MetadataCallback, RecordCallback

namespace databento {
// A reader for DBN files.
class DbnFileStore {
 public:
  explicit DbnFileStore(const std::string& file_path);
  DbnFileStore(const std::string& file_path,
               VersionUpgradePolicy upgrade_policy);

  void Replay(const MetadataCallback& metadata_callback,
              const RecordCallback& record_callback);
  void Replay(const RecordCallback& record_callback);

 private:
  DbnDecoder parser_;
};
}  // namespace databento
