// Record definitions from previous DBN versions and helper functions.
#pragma once

#include <cstddef>  // size_t
#include <cstdint>

#include "databento/v1.hpp"
#include "databento/v2.hpp"

namespace databento {
static constexpr std::size_t kSymbolCstrLenV1 = v1::kSymbolCstrLen;
static constexpr std::size_t kSymbolCstrLenV2 = v2::kSymbolCstrLen;

constexpr std::size_t VersionSymbolCstrLen(std::uint8_t version) {
  return version < 2 ? kSymbolCstrLenV1 : kSymbolCstrLenV2;
}

using InstrumentDefMsgV1 = v1::InstrumentDefMsg;
using InstrumentDefMsgV2 = v2::InstrumentDefMsg;
using ErrorMsgV1 = v1::ErrorMsg;
using ErrorMsgV2 = v2::ErrorMsg;
using SymbolMappingMsgV1 = v1::SymbolMappingMsg;
using SymbolMappingMsgV2 = v2::SymbolMappingMsg;
using SystemMsgV1 = v1::SystemMsg;
using SystemMsgV2 = v2::SystemMsg;
// DBN version 1 instrument definition.
}  // namespace databento
