#pragma once

#include <functional>  // function

#include "databento/dbn.hpp"     // Metadata
#include "databento/record.hpp"  // Record

namespace databento {
enum KeepGoing : std::uint8_t {
  Continue,
  Stop,
};

using MetadataCallback = std::function<void(Metadata&&)>;
using RecordCallback = std::function<KeepGoing(const Record&)>;
}  // namespace databento
