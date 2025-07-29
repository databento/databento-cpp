#pragma once

#include <cstddef>
#include <cstdint>

#include "databento/constants.hpp"  // kAssetCstrLen, kSymbolCstrLen, kUndefStatQuantity
#include "databento/record.hpp"

namespace databento::v3 {
static constexpr std::uint8_t kDbnVersion = 3;
static constexpr std::size_t kSymbolCstrLen = databento::kSymbolCstrLen;
static constexpr std::size_t kAssetCstrLen = databento::kAssetCstrLen;
static constexpr std::int64_t kUndefStatQuantity = databento::kUndefStatQuantity;

using MboMsg = databento::MboMsg;
using TradeMsg = databento::TradeMsg;
using Mbp1Msg = databento::Mbp1Msg;
using TbboMsg = databento::TbboMsg;
using Mbp10Msg = databento::Mbp10Msg;
using BboMsg = databento::BboMsg;
using Bbo1SMsg = databento::Bbo1SMsg;
using Bbo1MMsg = databento::Bbo1MMsg;
using Cmbp1Msg = databento::Cmbp1Msg;
using TcbboMsg = databento::TcbboMsg;
using CbboMsg = databento::CbboMsg;
using Cbbo1SMsg = databento::Cbbo1SMsg;
using Cbbo1MMsg = databento::Cbbo1MMsg;
using OhlcvMsg = databento::OhlcvMsg;
using StatusMsg = databento::StatusMsg;
using InstrumentDefMsg = databento::InstrumentDefMsg;
using ImbalanceMsg = databento::ImbalanceMsg;
using StatMsg = databento::StatMsg;
using ErrorMsg = databento::ErrorMsg;
using SymbolMappingMsg = databento::SymbolMappingMsg;
using SystemMsg = databento::SystemMsg;
}  // namespace databento::v3
