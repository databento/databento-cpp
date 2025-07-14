#pragma once

#include <cstdint>
#include <iosfwd>
#include <string>

namespace databento {
// A trading execution venue.
enum class Venue : std::uint16_t {
  // CME Globex
  Glbx = 1,
  // Nasdaq - All Markets
  Xnas = 2,
  // Nasdaq OMX BX
  Xbos = 3,
  // Nasdaq OMX PSX
  Xpsx = 4,
  // Cboe BZX U.S. Equities Exchange
  Bats = 5,
  // Cboe BYX U.S. Equities Exchange
  Baty = 6,
  // Cboe EDGA U.S. Equities Exchange
  Edga = 7,
  // Cboe EDGX U.S. Equities Exchange
  Edgx = 8,
  // New York Stock Exchange, Inc.
  Xnys = 9,
  // NYSE National, Inc.
  Xcis = 10,
  // NYSE MKT LLC
  Xase = 11,
  // NYSE Arca
  Arcx = 12,
  // NYSE Texas, Inc.
  Xchi = 13,
  // Investors Exchange
  Iexg = 14,
  // FINRA/Nasdaq TRF Carteret
  Finn = 15,
  // FINRA/Nasdaq TRF Chicago
  Finc = 16,
  // FINRA/NYSE TRF
  Finy = 17,
  // MEMX LLC Equities
  Memx = 18,
  // MIAX Pearl Equities
  Eprl = 19,
  // NYSE American Options
  Amxo = 20,
  // BOX Options
  Xbox = 21,
  // Cboe Options
  Xcbo = 22,
  // MIAX Emerald
  Emld = 23,
  // Cboe EDGX Options
  Edgo = 24,
  // Nasdaq GEMX
  Gmni = 25,
  // Nasdaq ISE
  Xisx = 26,
  // Nasdaq MRX
  Mcry = 27,
  // MIAX Options
  Xmio = 28,
  // NYSE Arca Options
  Arco = 29,
  // Options Price Reporting Authority
  Opra = 30,
  // MIAX Pearl
  Mprl = 31,
  // Nasdaq Options
  Xndq = 32,
  // Nasdaq BX Options
  Xbxo = 33,
  // Cboe C2 Options
  C2Ox = 34,
  // Nasdaq PHLX
  Xphl = 35,
  // Cboe BZX Options
  Bato = 36,
  // MEMX Options
  Mxop = 37,
  // ICE Europe Commodities
  Ifeu = 38,
  // ICE Endex
  Ndex = 39,
  // Databento US Equities - Consolidated
  Dbeq = 40,
  // MIAX Sapphire
  Sphr = 41,
  // Long-Term Stock Exchange, Inc.
  Ltse = 42,
  // Off-Exchange Transactions - Listed Instruments
  Xoff = 43,
  // IntelligentCross ASPEN Intelligent Bid/Offer
  Aspn = 44,
  // IntelligentCross ASPEN Maker/Taker
  Asmt = 45,
  // IntelligentCross ASPEN Inverted
  Aspi = 46,
  // Databento US Equities - Consolidated
  Equs = 47,
  // ICE Futures US
  Ifus = 48,
  // ICE Europe Financials
  Ifll = 49,
  // Eurex Exchange
  Xeur = 50,
  // European Energy Exchange
  Xeee = 51,
};

// A source of data.
enum class Dataset : std::uint16_t {
  // CME MDP 3.0 Market Data
  GlbxMdp3 = 1,
  // Nasdaq TotalView-ITCH
  XnasItch = 2,
  // Nasdaq BX TotalView-ITCH
  XbosItch = 3,
  // Nasdaq PSX TotalView-ITCH
  XpsxItch = 4,
  // Cboe BZX Depth
  BatsPitch = 5,
  // Cboe BYX Depth
  BatyPitch = 6,
  // Cboe EDGA Depth
  EdgaPitch = 7,
  // Cboe EDGX Depth
  EdgxPitch = 8,
  // NYSE Integrated
  XnysPillar = 9,
  // NYSE National Integrated
  XcisPillar = 10,
  // NYSE American Integrated
  XasePillar = 11,
  // NYSE Texas Integrated
  XchiPillar = 12,
  // NYSE National BBO
  XcisBbo = 13,
  // NYSE National Trades
  XcisTrades = 14,
  // MEMX Memoir Depth
  MemxMemoir = 15,
  // MIAX Pearl Depth
  EprlDom = 16,
  // FINRA/Nasdaq TRF (DEPRECATED)
  FinnNls = 17,
  // FINRA/NYSE TRF (DEPRECATED)
  FinyTrades = 18,
  // OPRA Binary
  OpraPillar = 19,
  // Databento US Equities Basic
  DbeqBasic = 20,
  // NYSE Arca Integrated
  ArcxPillar = 21,
  // IEX TOPS
  IexgTops = 22,
  // Databento US Equities Plus
  EqusPlus = 23,
  // NYSE BBO
  XnysBbo = 24,
  // NYSE Trades
  XnysTrades = 25,
  // Nasdaq QBBO
  XnasQbbo = 26,
  // Nasdaq NLS
  XnasNls = 27,
  // ICE Europe Commodities iMpact
  IfeuImpact = 28,
  // ICE Endex iMpact
  NdexImpact = 29,
  // Databento US Equities (All Feeds)
  EqusAll = 30,
  // Nasdaq Basic (NLS and QBBO)
  XnasBasic = 31,
  // Databento US Equities Summary
  EqusSummary = 32,
  // NYSE National Trades and BBO
  XcisTradesbbo = 33,
  // NYSE Trades and BBO
  XnysTradesbbo = 34,
  // Databento US Equities Mini
  EqusMini = 35,
  // ICE Futures US iMpact
  IfusImpact = 36,
  // ICE Europe Financials iMpact
  IfllImpact = 37,
  // Eurex EOBI
  XeurEobi = 38,
  // European Energy Exchange EOBI
  XeeeEobi = 39,
};

// A specific Venue from a specific data source.
enum class Publisher : std::uint16_t {
  // CME Globex MDP 3.0
  GlbxMdp3Glbx = 1,
  // Nasdaq TotalView-ITCH
  XnasItchXnas = 2,
  // Nasdaq BX TotalView-ITCH
  XbosItchXbos = 3,
  // Nasdaq PSX TotalView-ITCH
  XpsxItchXpsx = 4,
  // Cboe BZX Depth
  BatsPitchBats = 5,
  // Cboe BYX Depth
  BatyPitchBaty = 6,
  // Cboe EDGA Depth
  EdgaPitchEdga = 7,
  // Cboe EDGX Depth
  EdgxPitchEdgx = 8,
  // NYSE Integrated
  XnysPillarXnys = 9,
  // NYSE National Integrated
  XcisPillarXcis = 10,
  // NYSE American Integrated
  XasePillarXase = 11,
  // NYSE Texas Integrated
  XchiPillarXchi = 12,
  // NYSE National BBO
  XcisBboXcis = 13,
  // NYSE National Trades
  XcisTradesXcis = 14,
  // MEMX Memoir Depth
  MemxMemoirMemx = 15,
  // MIAX Pearl Depth
  EprlDomEprl = 16,
  // FINRA/Nasdaq TRF Carteret
  XnasNlsFinn = 17,
  // FINRA/Nasdaq TRF Chicago
  XnasNlsFinc = 18,
  // FINRA/NYSE TRF
  XnysTradesFiny = 19,
  // OPRA - NYSE American Options
  OpraPillarAmxo = 20,
  // OPRA - BOX Options
  OpraPillarXbox = 21,
  // OPRA - Cboe Options
  OpraPillarXcbo = 22,
  // OPRA - MIAX Emerald
  OpraPillarEmld = 23,
  // OPRA - Cboe EDGX Options
  OpraPillarEdgo = 24,
  // OPRA - Nasdaq GEMX
  OpraPillarGmni = 25,
  // OPRA - Nasdaq ISE
  OpraPillarXisx = 26,
  // OPRA - Nasdaq MRX
  OpraPillarMcry = 27,
  // OPRA - MIAX Options
  OpraPillarXmio = 28,
  // OPRA - NYSE Arca Options
  OpraPillarArco = 29,
  // OPRA - Options Price Reporting Authority
  OpraPillarOpra = 30,
  // OPRA - MIAX Pearl
  OpraPillarMprl = 31,
  // OPRA - Nasdaq Options
  OpraPillarXndq = 32,
  // OPRA - Nasdaq BX Options
  OpraPillarXbxo = 33,
  // OPRA - Cboe C2 Options
  OpraPillarC2Ox = 34,
  // OPRA - Nasdaq PHLX
  OpraPillarXphl = 35,
  // OPRA - Cboe BZX Options
  OpraPillarBato = 36,
  // OPRA - MEMX Options
  OpraPillarMxop = 37,
  // IEX TOPS
  IexgTopsIexg = 38,
  // DBEQ Basic - NYSE Texas
  DbeqBasicXchi = 39,
  // DBEQ Basic - NYSE National
  DbeqBasicXcis = 40,
  // DBEQ Basic - IEX
  DbeqBasicIexg = 41,
  // DBEQ Basic - MIAX Pearl
  DbeqBasicEprl = 42,
  // NYSE Arca Integrated
  ArcxPillarArcx = 43,
  // NYSE BBO
  XnysBboXnys = 44,
  // NYSE Trades
  XnysTradesXnys = 45,
  // Nasdaq QBBO
  XnasQbboXnas = 46,
  // Nasdaq Trades
  XnasNlsXnas = 47,
  // Databento US Equities Plus - NYSE Texas
  EqusPlusXchi = 48,
  // Databento US Equities Plus - NYSE National
  EqusPlusXcis = 49,
  // Databento US Equities Plus - IEX
  EqusPlusIexg = 50,
  // Databento US Equities Plus - MIAX Pearl
  EqusPlusEprl = 51,
  // Databento US Equities Plus - Nasdaq
  EqusPlusXnas = 52,
  // Databento US Equities Plus - NYSE
  EqusPlusXnys = 53,
  // Databento US Equities Plus - FINRA/Nasdaq TRF Carteret
  EqusPlusFinn = 54,
  // Databento US Equities Plus - FINRA/NYSE TRF
  EqusPlusFiny = 55,
  // Databento US Equities Plus - FINRA/Nasdaq TRF Chicago
  EqusPlusFinc = 56,
  // ICE Europe Commodities
  IfeuImpactIfeu = 57,
  // ICE Endex
  NdexImpactNdex = 58,
  // Databento US Equities Basic - Consolidated
  DbeqBasicDbeq = 59,
  // EQUS Plus - Consolidated
  EqusPlusEqus = 60,
  // OPRA - MIAX Sapphire
  OpraPillarSphr = 61,
  // Databento US Equities (All Feeds) - NYSE Texas
  EqusAllXchi = 62,
  // Databento US Equities (All Feeds) - NYSE National
  EqusAllXcis = 63,
  // Databento US Equities (All Feeds) - IEX
  EqusAllIexg = 64,
  // Databento US Equities (All Feeds) - MIAX Pearl
  EqusAllEprl = 65,
  // Databento US Equities (All Feeds) - Nasdaq
  EqusAllXnas = 66,
  // Databento US Equities (All Feeds) - NYSE
  EqusAllXnys = 67,
  // Databento US Equities (All Feeds) - FINRA/Nasdaq TRF Carteret
  EqusAllFinn = 68,
  // Databento US Equities (All Feeds) - FINRA/NYSE TRF
  EqusAllFiny = 69,
  // Databento US Equities (All Feeds) - FINRA/Nasdaq TRF Chicago
  EqusAllFinc = 70,
  // Databento US Equities (All Feeds) - Cboe BZX
  EqusAllBats = 71,
  // Databento US Equities (All Feeds) - Cboe BYX
  EqusAllBaty = 72,
  // Databento US Equities (All Feeds) - Cboe EDGA
  EqusAllEdga = 73,
  // Databento US Equities (All Feeds) - Cboe EDGX
  EqusAllEdgx = 74,
  // Databento US Equities (All Feeds) - Nasdaq BX
  EqusAllXbos = 75,
  // Databento US Equities (All Feeds) - Nasdaq PSX
  EqusAllXpsx = 76,
  // Databento US Equities (All Feeds) - MEMX
  EqusAllMemx = 77,
  // Databento US Equities (All Feeds) - NYSE American
  EqusAllXase = 78,
  // Databento US Equities (All Feeds) - NYSE Arca
  EqusAllArcx = 79,
  // Databento US Equities (All Feeds) - Long-Term Stock Exchange
  EqusAllLtse = 80,
  // Nasdaq Basic - Nasdaq
  XnasBasicXnas = 81,
  // Nasdaq Basic - FINRA/Nasdaq TRF Carteret
  XnasBasicFinn = 82,
  // Nasdaq Basic - FINRA/Nasdaq TRF Chicago
  XnasBasicFinc = 83,
  // ICE Europe - Off-Market Trades
  IfeuImpactXoff = 84,
  // ICE Endex - Off-Market Trades
  NdexImpactXoff = 85,
  // Nasdaq NLS - Nasdaq BX
  XnasNlsXbos = 86,
  // Nasdaq NLS - Nasdaq PSX
  XnasNlsXpsx = 87,
  // Nasdaq Basic - Nasdaq BX
  XnasBasicXbos = 88,
  // Nasdaq Basic - Nasdaq PSX
  XnasBasicXpsx = 89,
  // Databento Equities Summary
  EqusSummaryEqus = 90,
  // NYSE National Trades and BBO
  XcisTradesbboXcis = 91,
  // NYSE Trades and BBO
  XnysTradesbboXnys = 92,
  // Nasdaq Basic - Consolidated
  XnasBasicEqus = 93,
  // Databento US Equities (All Feeds) - Consolidated
  EqusAllEqus = 94,
  // Databento US Equities Mini
  EqusMiniEqus = 95,
  // NYSE Trades - Consolidated
  XnysTradesEqus = 96,
  // ICE Futures US
  IfusImpactIfus = 97,
  // ICE Futures US - Off-Market Trades
  IfusImpactXoff = 98,
  // ICE Europe Financials
  IfllImpactIfll = 99,
  // ICE Europe Financials - Off-Market Trades
  IfllImpactXoff = 100,
  // Eurex EOBI
  XeurEobiXeur = 101,
  // European Energy Exchange EOBI
  XeeeEobiXeee = 102,
  // Eurex EOBI - Off-Market Trades
  XeurEobiXoff = 103,
  // European Energy Exchange EOBI - Off-Market Trades
  XeeeEobiXoff = 104,
};

// Get a Publisher's Venue.
Venue PublisherVenue(Publisher publisher);

// Get a Publisher's Dataset.
Dataset PublisherDataset(Publisher publisher);

template <typename T>
T FromString(const std::string& str);
const char* ToString(Venue venue);
std::ostream& operator<<(std::ostream& out, Venue venue);
template <>
Venue FromString(const std::string& str);

const char* ToString(Dataset dataset);
std::ostream& operator<<(std::ostream& out, Dataset dataset);
template <>
Dataset FromString(const std::string& str);

const char* ToString(Publisher publisher);
std::ostream& operator<<(std::ostream& out, Publisher publisher);
template <>
Publisher FromString(const std::string& str);
}  // namespace databento
