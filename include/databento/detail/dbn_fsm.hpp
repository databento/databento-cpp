#pragma once

#include <cstddef>  // byte, size_t
#include <cstdint>  // uint8_t
#include <memory>   // unique_ptr
#include <optional>

#include "databento/dbn.hpp"     // Metadata
#include "databento/enums.hpp"   // VersionUpgradePolicy
#include "databento/record.hpp"  // Record

namespace databento::detail {
struct CFfiDecoder;

void FreeCFfiDecoder(CFfiDecoder* decoder);

// A push-based DBN decoder backed by the DBN library. Bytes are pushed in with
// `Space` and `Fill` or with `WriteAll`, and decoded metadata and records are
// drained out with `Process`. Compressed data must be decompressed before it's
// pushed in.
class DbnFsm {
 public:
  enum class Status : std::uint8_t {
    // More data is needed before anything else can be decoded.
    ReadMore,
    // Decoded the metadata, which `TakeMetadata` returns.
    Metadata,
    // Decoded a record, which `LastRecord` returns.
    Record,
  };

  explicit DbnFsm(VersionUpgradePolicy upgrade_policy);
  DbnFsm(VersionUpgradePolicy upgrade_policy, std::size_t buffer_size);

  // Returns space to read up to `*length` bytes into, at least the size of the
  // largest record. Call `Fill` with the number of bytes read.
  std::byte* Space(std::size_t* length);
  // Indicates how many bytes were written to the space from `Space`.
  void Fill(std::size_t length);
  // Copies data in. A copying alternative to `Space` and `Fill`.
  void WriteAll(const char* data, std::size_t length);
  void WriteAll(const std::byte* data, std::size_t length);
  // Decodes what's buffered. Should be called repeatedly until it returns
  // `ReadMore`, at which point more data should be pushed in.
  Status Process();
  // Moves out the metadata from the last `Process` that returned
  // `Status::Metadata`. Throws an `Exception` if there is none.
  Metadata TakeMetadata();
  // The record from the last `Process` that returned `Status::Record`. It's valid
  // until the next call to `Process`, `Space`, or `WriteAll`.
  const Record& LastRecord() const { return last_record_; }
  // The number of buffered bytes that haven't been decoded.
  std::size_t UnreadBytes() const;
  // Resets to expect DBN metadata so the same decoder can decode another stream.
  void Reset();

 private:
  std::unique_ptr<CFfiDecoder, void (*)(CFfiDecoder*)> decoder_;
  // Holds the decoded metadata until `TakeMetadata` moves it out
  std::optional<databento::Metadata> metadata_;
  // Owns the `Record` view of the library's buffer so callers can be handed a
  // reference that outlives the call
  databento::Record last_record_{nullptr};
};
}  // namespace databento::detail
