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
  // NYSE Chicago, Inc.
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
  // BOX Options Exchange
  Xbox = 21,
  // Cboe Options Exchange
  Xcbo = 22,
  // MIAX Emerald
  Emld = 23,
  // Cboe EDGX Options Exchange
  Edgo = 24,
  // ISE Gemini Exchange
  Gmni = 25,
  // International Securities Exchange, LLC
  Xisx = 26,
  // ISE Mercury, LLC
  Mcry = 27,
  // Miami International Securities Exchange
  Xmio = 28,
  // NYSE Arca Options
  Arco = 29,
  // Options Price Reporting Authority
  Opra = 30,
  // MIAX Pearl
  Mprl = 31,
  // Nasdaq Options Market
  Xndq = 32,
  // Nasdaq OMX BX Options
  Xbxo = 33,
  // Cboe C2 Options Exchange
  C2Ox = 34,
  // Nasdaq OMX PHLX
  Xphl = 35,
  // Cboe BZX Options Exchange
  Bato = 36,
  // MEMX LLC Options
  Mxop = 37,
  // ICE Futures Europe (Commodities)
  Ifeu = 38,
  // ICE Endex
  Ndex = 39,
  // Databento Equities - Consolidated
  Dbeq = 40,
  // MIAX Sapphire
  Sphr = 41,
  // Long-Term Stock Exchange, Inc.
  Ltse = 42,
  // Off-Exchange Transactions - Listed Instruments
  Xoff = 43,
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
  // Cboe BZX Depth Pitch
  BatsPitch = 5,
  // Cboe BYX Depth Pitch
  BatyPitch = 6,
  // Cboe EDGA Depth Pitch
  EdgaPitch = 7,
  // Cboe EDGX Depth Pitch
  EdgxPitch = 8,
  // NYSE Integrated
  XnysPillar = 9,
  // NYSE National Integrated
  XcisPillar = 10,
  // NYSE American Integrated
  XasePillar = 11,
  // NYSE Chicago Integrated
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
  // Databento Equities Basic
  DbeqBasic = 20,
  // NYSE Arca Integrated
  ArcxPillar = 21,
  // IEX TOPS
  IexgTops = 22,
  // Databento Equities Plus
  DbeqPlus = 23,
  // NYSE BBO
  XnysBbo = 24,
  // NYSE Trades
  XnysTrades = 25,
  // Nasdaq QBBO
  XnasQbbo = 26,
  // Nasdaq NLS
  XnasNls = 27,
  // ICE Futures Europe (Commodities) iMpact
  IfeuImpact = 28,
  // ICE Endex iMpact
  NdexImpact = 29,
  // Databento Equities Max
  DbeqMax = 30,
  // Nasdaq Basic (NLS+QBBO)
  XnasBasic = 31,
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
  // Cboe BZX Depth Pitch
  BatsPitchBats = 5,
  // Cboe BYX Depth Pitch
  BatyPitchBaty = 6,
  // Cboe EDGA Depth Pitch
  EdgaPitchEdga = 7,
  // Cboe EDGX Depth Pitch
  EdgxPitchEdgx = 8,
  // NYSE Integrated
  XnysPillarXnys = 9,
  // NYSE National Integrated
  XcisPillarXcis = 10,
  // NYSE American Integrated
  XasePillarXase = 11,
  // NYSE Chicago Integrated
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
  // OPRA - NYSE American
  OpraPillarAmxo = 20,
  // OPRA - Boston Options Exchange
  OpraPillarXbox = 21,
  // OPRA - Cboe Options Exchange
  OpraPillarXcbo = 22,
  // OPRA - MIAX Emerald
  OpraPillarEmld = 23,
  // OPRA - Cboe EDGX Options Exchange
  OpraPillarEdgo = 24,
  // OPRA - Nasdaq GEMX
  OpraPillarGmni = 25,
  // OPRA - Nasdaq ISE
  OpraPillarXisx = 26,
  // OPRA - Nasdaq MRX
  OpraPillarMcry = 27,
  // OPRA - Miami International Securities
  OpraPillarXmio = 28,
  // OPRA - NYSE Arca
  OpraPillarArco = 29,
  // OPRA - Options Price Reporting Authority
  OpraPillarOpra = 30,
  // OPRA - MIAX Pearl
  OpraPillarMprl = 31,
  // OPRA - Nasdaq Options Market
  OpraPillarXndq = 32,
  // OPRA - Nasdaq BX Options
  OpraPillarXbxo = 33,
  // OPRA - Cboe C2 Options Exchange
  OpraPillarC2Ox = 34,
  // OPRA - Nasdaq PHLX
  OpraPillarXphl = 35,
  // OPRA - Cboe BZX Options
  OpraPillarBato = 36,
  // OPRA - MEMX Options Exchange
  OpraPillarMxop = 37,
  // IEX TOPS
  IexgTopsIexg = 38,
  // DBEQ Basic - NYSE Chicago
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
  // DBEQ Plus - NYSE Chicago
  DbeqPlusXchi = 48,
  // DBEQ Plus - NYSE National
  DbeqPlusXcis = 49,
  // DBEQ Plus - IEX
  DbeqPlusIexg = 50,
  // DBEQ Plus - MIAX Pearl
  DbeqPlusEprl = 51,
  // DBEQ Plus - Nasdaq
  DbeqPlusXnas = 52,
  // DBEQ Plus - NYSE
  DbeqPlusXnys = 53,
  // DBEQ Plus - FINRA/Nasdaq TRF Carteret
  DbeqPlusFinn = 54,
  // DBEQ Plus - FINRA/NYSE TRF
  DbeqPlusFiny = 55,
  // DBEQ Plus - FINRA/Nasdaq TRF Chicago
  DbeqPlusFinc = 56,
  // ICE Futures Europe (Commodities)
  IfeuImpactIfeu = 57,
  // ICE Endex
  NdexImpactNdex = 58,
  // DBEQ Basic - Consolidated
  DbeqBasicDbeq = 59,
  // DBEQ Plus - Consolidated
  DbeqPlusDbeq = 60,
  // OPRA - MIAX Sapphire
  OpraPillarSphr = 61,
  // DBEQ Max - NYSE Chicago
  DbeqMaxXchi = 62,
  // DBEQ Max - NYSE National
  DbeqMaxXcis = 63,
  // DBEQ Max - IEX
  DbeqMaxIexg = 64,
  // DBEQ Max - MIAX Pearl
  DbeqMaxEprl = 65,
  // DBEQ Max - Nasdaq
  DbeqMaxXnas = 66,
  // DBEQ Max - NYSE
  DbeqMaxXnys = 67,
  // DBEQ Max - FINRA/Nasdaq TRF Carteret
  DbeqMaxFinn = 68,
  // DBEQ Max - FINRA/NYSE TRF
  DbeqMaxFiny = 69,
  // DBEQ Max - FINRA/Nasdaq TRF Chicago
  DbeqMaxFinc = 70,
  // DBEQ Max - CBOE BZX
  DbeqMaxBats = 71,
  // DBEQ Max - CBOE BYX
  DbeqMaxBaty = 72,
  // DBEQ Max - CBOE EDGA
  DbeqMaxEdga = 73,
  // DBEQ Max - CBOE EDGX
  DbeqMaxEdgx = 74,
  // DBEQ Max - Nasdaq BX
  DbeqMaxXbos = 75,
  // DBEQ Max - Nasdaq PSX
  DbeqMaxXpsx = 76,
  // DBEQ Max - MEMX
  DbeqMaxMemx = 77,
  // DBEQ Max - NYSE American
  DbeqMaxXase = 78,
  // DBEQ Max - NYSE Arca
  DbeqMaxArcx = 79,
  // DBEQ Max - Long-Term Stock Exchange
  DbeqMaxLtse = 80,
  // Nasdaq Basic - Nasdaq
  XnasBasicXnas = 81,
  // Nasdaq Basic - FINRA/Nasdaq TRF Carteret
  XnasBasicFinn = 82,
  // Nasdaq Basic - FINRA/Nasdaq TRF Chicago
  XnasBasicFinc = 83,
  // ICE Futures Europe - Off-Market Trades
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
