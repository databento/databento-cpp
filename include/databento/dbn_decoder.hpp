#pragma once

#include <cstddef>  // size_t
#include <memory>   // unique_ptr

#include "databento/dbn.hpp"
#include "databento/detail/dbn_fsm.hpp"
#include "databento/enums.hpp"  // Upgrade Policy
#include "databento/file_stream.hpp"
#include "databento/ireadable.hpp"
#include "databento/log.hpp"
#include "databento/record.hpp"  // Record

namespace databento {
// DBN decoder. Set upgrade_policy to control how DBN version 1 data should be
// handled. Defaults to upgrading DBN versions 1 and 2 to version 3 (the current
// version).
class DbnDecoder {
 public:
  DbnDecoder(ILogReceiver* log_receiver, InFileStream file_stream);
  DbnDecoder(ILogReceiver* log_receiver, std::unique_ptr<IReadable> input);
  DbnDecoder(ILogReceiver* log_receiver, std::unique_ptr<IReadable> input,
             VersionUpgradePolicy upgrade_policy);

  // Should be called exactly once.
  Metadata DecodeMetadata();
  // Lifetime of returned Record is until next call to DecodeRecord. Returns
  // nullptr once the end of the input has been reached.
  const Record* DecodeRecord();

 private:
  void DetectCompression();
  std::size_t FillBuffer();

  ILogReceiver* log_receiver_;
  std::unique_ptr<IReadable> input_;
  detail::DbnFsm fsm_;
};
}  // namespace databento
