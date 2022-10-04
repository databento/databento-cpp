#pragma once

#include <functional>  // function

#include "databento/dbz.hpp"     // Metadata
#include "databento/record.hpp"  // Record

namespace databento {
enum KeepGoing : std::uint8_t {
  Continue,
  Stop,
};

using MetadataCallback = std::function<void(Metadata)>;
using RecordCallback = std::function<KeepGoing(const Record&)>;
}  // namespace databento
