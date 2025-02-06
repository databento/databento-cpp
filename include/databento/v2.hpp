#pragma once

#include "databento/constants.hpp"  // kSymbolCstrLen
#include "databento/record.hpp"

namespace databento::v2 {
static constexpr std::size_t kSymbolCstrLen = databento::kSymbolCstrLen;

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
}  // namespace databento::v2
