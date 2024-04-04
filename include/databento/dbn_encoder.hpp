#pragma once

#include "databento/dbn.hpp"
#include "databento/iwritable.hpp"

namespace databento {

class DbnEncoder {
 public:
  // Encode metadata from the given buffer.
  static void EncodeMetadata(const Metadata& buffer, IWritable& writer);
};
}  // namespace databento
