#pragma once

#include <cstdint>
#include <iosfwd>
#include <string>

namespace databento {
// A trading execution venue.
enum class Venue : std::uint16_t {
  // CME GLOBEX
  Glbx = 1,
  // NASDAQ
  Xnas = 2,
  // NASDAQ OMX BX
  Xbos = 3,
  // NASDAQ OMX PSX
  Xpsx = 4,
  // CBOE BZX U.S. EQUITIES EXCHANGE
  Bats = 5,
  // CBOE BYX U.S. EQUITIES EXCHANGE
  Baty = 6,
  // CBOE EDGA U.S. EQUITIES EXCHANGE
  Edga = 7,
  // CBOE EDGX U.S. EQUITIES EXCHANGE
  Edgx = 8,
  // New York Stock Exchange
  Xnys = 9,
  // NYSE NATIONAL, INC.
  Xcis = 10,
  // NYSE AMERICAN
  Xase = 11,
  // NYSE ARCA
  Arcx = 12,
  // NYSE CHICAGO, INC.
  Xchi = 13,
  // INVESTORS EXCHANGE
  Iexg = 14,
  // FINRA/NASDAQ TRF CARTERET
  Finn = 15,
  // FINRA/NASDAQ TRF CHICAGO
  Finc = 16,
  // FINRA/NYSE TRF
  Finy = 17,
  // MEMX LLC EQUITIES
  Memx = 18,
  // MIAX PEARL EQUITIES
  Eprl = 19,
  // NYSE AMERICAN OPTIONS
  Amxo = 20,
  // BOX OPTIONS EXCHANGE
  Xbox = 21,
  // CBOE OPTIONS EXCHANGE
  Xcbo = 22,
  // MIAX EMERALD
  Emld = 23,
  // Cboe EDGX Options Exchange
  Edgo = 24,
  // NASDAQ GEMX
  Gmni = 25,
  // NASDAQ ISE
  Xisx = 26,
  // NASDAQ MRX
  Mcry = 27,
  // MIAX INTERNATIONAL SECURITIES
  Xmio = 28,
  // NYSE ARCA OPTIONS
  Arco = 29,
  // OPRA
  Opra = 30,
  // MIAX PEARL
  Mprl = 31,
  // NASDAQ OPTIONS MARKET
  Xndq = 32,
  // NASDAQ BX OPTIONS
  Xbxo = 33,
  // CBOE C2 OPTIONS EXCHANGE
  C2Ox = 34,
  // NASDAQ PHLX
  Xphl = 35,
  // CBOE BZX Options Exchange
  Bato = 36,
  // MEMX Options Exchange
  Mxop = 37,
};

// A source of data.
enum class Dataset : std::uint16_t {
  // CME MDP 3.0 Market Data
  GlbxMdp3 = 1,
  // Nasdaq XNAS TotalView-ITCH
  XnasItch = 2,
  // Nasdaq XBOS TotalView-ITCH
  XbosItch = 3,
  // Nasdaq XPSX TotalView-ITCH
  XpsxItch = 4,
  // CBOE BZX
  BatsPitch = 5,
  // CBOE BYX
  BatyPitch = 6,
  // CBOE EDGA
  EdgaPitch = 7,
  // CBOE EDGX
  EdgxPitch = 8,
  // NYSE
  XnysPillar = 9,
  // NYSE National
  XcisPillar = 10,
  // NYSE American
  XasePillar = 11,
  // NYSE Chicago
  XchiPillar = 12,
  // NYSE National BBO
  XcisBbo = 13,
  // NYSE National TRADES
  XcisTrades = 14,
  // MEMX Memoir Depth
  MemxMemoir = 15,
  // MIAX Pearl Depth
  EprlDom = 16,
  // Finra/Nasdaq TRF
  FinnNls = 17,
  // Finra/NYSE TRF
  FinyTrades = 18,
  // OPRA Binary Recipient
  OpraPillar = 19,
  // Databento Equities Basic
  DbeqBasic = 20,
  // NYSE Arca
  ArcxPillar = 21,
  // Investors Exchange TOPS
  IexgTops = 22,
};

// A specific Venue from a specific data source.
enum class Publisher : std::uint16_t {
  // CME Globex MDP 3.0
  GlbxMdp3Glbx = 1,
  // Nasdaq TotalView ITCH
  XnasItchXnas = 2,
  // Nasdaq XBOS TotalView ITCH
  XbosItchXbos = 3,
  // Nasdaq XPSX TotalView ITCH
  XpsxItchXpsx = 4,
  // CBOE BZX
  BatsPitchBats = 5,
  // CBOE BYX
  BatyPitchBats = 6,
  // CBOE EDGA
  EdgaPitchEdga = 7,
  // CBOE EDGX
  EdgxPitchEdgx = 8,
  // NYSE
  XnysPillarXnys = 9,
  // NYSE National
  XcisPillarXcis = 10,
  // NYSE American
  XasePillarXase = 11,
  // NYSE Chicago
  XchiPillarXchi = 12,
  // NYSE National BBO
  XcisBboXcis = 13,
  // NYSE National Trades
  XcisTradesXcis = 14,
  // MEMX Memoir Depth
  MemxMemoirMemx = 15,
  // MIAX Pearl Depth
  EprlDomEprl = 16,
  // FINRA/NASDAQ TRF CARTERET
  FinnNlsFinn = 17,
  // FINRA/NASDAQ TRF CHICAGO
  FinnNlsFinc = 18,
  // FINRA/NYSE TRF
  FinyTradesFiny = 19,
  // OPRA - NYSE AMERICAN OPTIONS
  OpraPillarAmxo = 20,
  // OPRA - BOX OPTIONS EXCHANGE
  OpraPillarXbox = 21,
  // OPRA - CBOE OPTIONS EXCHANGE
  OpraPillarXcbo = 22,
  // OPRA - MIAX EMERALD
  OpraPillarEmld = 23,
  // OPRA - Cboe EDGX Options Exchange
  OpraPillarEdgo = 24,
  // OPRA - NASDAQ GEMX
  OpraPillarGmni = 25,
  // OPRA - NASDAQ ISE
  OpraPillarXisx = 26,
  // OPRA - NASDAQ MRX
  OpraPillarMcry = 27,
  // OPRA - MIAX INTERNATIONAL SECURITIES
  OpraPillarXmio = 28,
  // OPRA - NYSE ARCA OPTIONS
  OpraPillarArco = 29,
  // OPRA - OPRA
  OpraPillarOpra = 30,
  // OPRA - MIAX PEARL
  OpraPillarMprl = 31,
  // OPRA - NASDAQ OPTIONS MARKET
  OpraPillarXndq = 32,
  // OPRA - NASDAQ BX OPTIONS
  OpraPillarXbxo = 33,
  // OPRA - CBOE C2 OPTIONS EXCHANGE
  OpraPillarC2Ox = 34,
  // OPRA - NASDAQ PHLX
  OpraPillarXphl = 35,
  // OPRA - CBOE BZX Options Exchange
  OpraPillarBato = 36,
  // OPRA - MEMX OPTIONS EXCHANGE
  OpraPillarMxop = 37,
  // Investors Exchange TOPS
  IexgTopsIexg = 38,
  // DBEQ Basic - Nyse Chicago
  DbeqBasicXchi = 39,
  // DBEQ Basic - Nyse National
  DbeqBasicXcis = 40,
  // DBEQ Basic - Investors Exchange
  DbeqBasicIexg = 41,
  // DBEQ Basic - MIAX Pearl
  DbeqBasicEprl = 42,
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
