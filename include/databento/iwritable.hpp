#pragma once

#include <cstddef>  // byte, size_t

namespace databento {
// An abstract class for writable objects to allow for runtime polymorphism
// around DBN encoding.
class IWritable {
 public:
  virtual ~IWritable() = default;

  virtual void WriteAll(const std::byte* buffer, std::size_t length) = 0;
};
}  // namespace databento
