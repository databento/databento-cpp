#pragma once

#include <memory>  // unique_ptr
#include <string>

#include "databento/dbz.hpp"         // Metadata
#include "databento/dbz_parser.hpp"  // DbzFileParser
#include "databento/timeseries.hpp"  // MetadataCallback, RecordCallback

namespace databento {
// A reader for DBZ files.
class FileBento {
 public:
  explicit FileBento(const std::string& file_path);

  void Replay(const MetadataCallback& metadata_callback,
              const RecordCallback& record_callback);
  void Replay(const RecordCallback& record_callback);

 private:
  DbzFileParser parser_;
};
}  // namespace databento
