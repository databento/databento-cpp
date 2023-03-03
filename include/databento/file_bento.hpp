#pragma once

#include <memory>  // unique_ptr
#include <string>

#include "databento/dbn.hpp"          // Metadata
#include "databento/dbn_decoder.hpp"  // DbnDecoder
#include "databento/timeseries.hpp"   // MetadataCallback, RecordCallback

namespace databento {
// A reader for DBN files.
class FileBento {
 public:
  explicit FileBento(const std::string& file_path);

  void Replay(const MetadataCallback& metadata_callback,
              const RecordCallback& record_callback);
  void Replay(const RecordCallback& record_callback);

 private:
  DbnDecoder parser_;
};
}  // namespace databento
