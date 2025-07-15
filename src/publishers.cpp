#include "databento/publishers.hpp"

#include <cstdint>
#include <ostream>
#include <string>

#include "databento/exceptions.hpp"  // InvalidArgumentError

namespace databento {
// NOLINTBEGIN(bugprone-branch-clone)

const char* ToString(Venue venue) {
  switch (venue) {
    case Venue::Glbx: {
      return "GLBX";
    }
    case Venue::Xnas: {
      return "XNAS";
    }
    case Venue::Xbos: {
      return "XBOS";
    }
    case Venue::Xpsx: {
      return "XPSX";
    }
    case Venue::Bats: {
      return "BATS";
    }
    case Venue::Baty: {
      return "BATY";
    }
    case Venue::Edga: {
      return "EDGA";
    }
    case Venue::Edgx: {
      return "EDGX";
    }
    case Venue::Xnys: {
      return "XNYS";
    }
    case Venue::Xcis: {
      return "XCIS";
    }
    case Venue::Xase: {
      return "XASE";
    }
    case Venue::Arcx: {
      return "ARCX";
    }
    case Venue::Xchi: {
      return "XCHI";
    }
    case Venue::Iexg: {
      return "IEXG";
    }
    case Venue::Finn: {
      return "FINN";
    }
    case Venue::Finc: {
      return "FINC";
    }
    case Venue::Finy: {
      return "FINY";
    }
    case Venue::Memx: {
      return "MEMX";
    }
    case Venue::Eprl: {
      return "EPRL";
    }
    case Venue::Amxo: {
      return "AMXO";
    }
    case Venue::Xbox: {
      return "XBOX";
    }
    case Venue::Xcbo: {
      return "XCBO";
    }
    case Venue::Emld: {
      return "EMLD";
    }
    case Venue::Edgo: {
      return "EDGO";
    }
    case Venue::Gmni: {
      return "GMNI";
    }
    case Venue::Xisx: {
      return "XISX";
    }
    case Venue::Mcry: {
      return "MCRY";
    }
    case Venue::Xmio: {
      return "XMIO";
    }
    case Venue::Arco: {
      return "ARCO";
    }
    case Venue::Opra: {
      return "OPRA";
    }
    case Venue::Mprl: {
      return "MPRL";
    }
    case Venue::Xndq: {
      return "XNDQ";
    }
    case Venue::Xbxo: {
      return "XBXO";
    }
    case Venue::C2Ox: {
      return "C2OX";
    }
    case Venue::Xphl: {
      return "XPHL";
    }
    case Venue::Bato: {
      return "BATO";
    }
    case Venue::Mxop: {
      return "MXOP";
    }
    case Venue::Ifeu: {
      return "IFEU";
    }
    case Venue::Ndex: {
      return "NDEX";
    }
    case Venue::Dbeq: {
      return "DBEQ";
    }
    case Venue::Sphr: {
      return "SPHR";
    }
    case Venue::Ltse: {
      return "LTSE";
    }
    case Venue::Xoff: {
      return "XOFF";
    }
    case Venue::Aspn: {
      return "ASPN";
    }
    case Venue::Asmt: {
      return "ASMT";
    }
    case Venue::Aspi: {
      return "ASPI";
    }
    case Venue::Equs: {
      return "EQUS";
    }
    case Venue::Ifus: {
      return "IFUS";
    }
    case Venue::Ifll: {
      return "IFLL";
    }
    case Venue::Xeur: {
      return "XEUR";
    }
    case Venue::Xeee: {
      return "XEEE";
    }
    default: {
      return "Unknown";
    }
  }
}

std::ostream& operator<<(std::ostream& out, Venue venue) {
  out << ToString(venue);
  return out;
}

template <>
Venue FromString(const std::string& str) {
  if (str == "GLBX") {
    return Venue::Glbx;
  }
  if (str == "XNAS") {
    return Venue::Xnas;
  }
  if (str == "XBOS") {
    return Venue::Xbos;
  }
  if (str == "XPSX") {
    return Venue::Xpsx;
  }
  if (str == "BATS") {
    return Venue::Bats;
  }
  if (str == "BATY") {
    return Venue::Baty;
  }
  if (str == "EDGA") {
    return Venue::Edga;
  }
  if (str == "EDGX") {
    return Venue::Edgx;
  }
  if (str == "XNYS") {
    return Venue::Xnys;
  }
  if (str == "XCIS") {
    return Venue::Xcis;
  }
  if (str == "XASE") {
    return Venue::Xase;
  }
  if (str == "ARCX") {
    return Venue::Arcx;
  }
  if (str == "XCHI") {
    return Venue::Xchi;
  }
  if (str == "IEXG") {
    return Venue::Iexg;
  }
  if (str == "FINN") {
    return Venue::Finn;
  }
  if (str == "FINC") {
    return Venue::Finc;
  }
  if (str == "FINY") {
    return Venue::Finy;
  }
  if (str == "MEMX") {
    return Venue::Memx;
  }
  if (str == "EPRL") {
    return Venue::Eprl;
  }
  if (str == "AMXO") {
    return Venue::Amxo;
  }
  if (str == "XBOX") {
    return Venue::Xbox;
  }
  if (str == "XCBO") {
    return Venue::Xcbo;
  }
  if (str == "EMLD") {
    return Venue::Emld;
  }
  if (str == "EDGO") {
    return Venue::Edgo;
  }
  if (str == "GMNI") {
    return Venue::Gmni;
  }
  if (str == "XISX") {
    return Venue::Xisx;
  }
  if (str == "MCRY") {
    return Venue::Mcry;
  }
  if (str == "XMIO") {
    return Venue::Xmio;
  }
  if (str == "ARCO") {
    return Venue::Arco;
  }
  if (str == "OPRA") {
    return Venue::Opra;
  }
  if (str == "MPRL") {
    return Venue::Mprl;
  }
  if (str == "XNDQ") {
    return Venue::Xndq;
  }
  if (str == "XBXO") {
    return Venue::Xbxo;
  }
  if (str == "C2OX") {
    return Venue::C2Ox;
  }
  if (str == "XPHL") {
    return Venue::Xphl;
  }
  if (str == "BATO") {
    return Venue::Bato;
  }
  if (str == "MXOP") {
    return Venue::Mxop;
  }
  if (str == "IFEU") {
    return Venue::Ifeu;
  }
  if (str == "NDEX") {
    return Venue::Ndex;
  }
  if (str == "DBEQ") {
    return Venue::Dbeq;
  }
  if (str == "SPHR") {
    return Venue::Sphr;
  }
  if (str == "LTSE") {
    return Venue::Ltse;
  }
  if (str == "XOFF") {
    return Venue::Xoff;
  }
  if (str == "ASPN") {
    return Venue::Aspn;
  }
  if (str == "ASMT") {
    return Venue::Asmt;
  }
  if (str == "ASPI") {
    return Venue::Aspi;
  }
  if (str == "EQUS") {
    return Venue::Equs;
  }
  if (str == "IFUS") {
    return Venue::Ifus;
  }
  if (str == "IFLL") {
    return Venue::Ifll;
  }
  if (str == "XEUR") {
    return Venue::Xeur;
  }
  if (str == "XEEE") {
    return Venue::Xeee;
  }
  throw InvalidArgumentError{"FromString<Venue>", "str",
                             "unknown value '" + str + '\''};
}

const char* ToString(Dataset dataset) {
  switch (dataset) {
    case Dataset::GlbxMdp3: {
      return "GLBX.MDP3";
    }
    case Dataset::XnasItch: {
      return "XNAS.ITCH";
    }
    case Dataset::XbosItch: {
      return "XBOS.ITCH";
    }
    case Dataset::XpsxItch: {
      return "XPSX.ITCH";
    }
    case Dataset::BatsPitch: {
      return "BATS.PITCH";
    }
    case Dataset::BatyPitch: {
      return "BATY.PITCH";
    }
    case Dataset::EdgaPitch: {
      return "EDGA.PITCH";
    }
    case Dataset::EdgxPitch: {
      return "EDGX.PITCH";
    }
    case Dataset::XnysPillar: {
      return "XNYS.PILLAR";
    }
    case Dataset::XcisPillar: {
      return "XCIS.PILLAR";
    }
    case Dataset::XasePillar: {
      return "XASE.PILLAR";
    }
    case Dataset::XchiPillar: {
      return "XCHI.PILLAR";
    }
    case Dataset::XcisBbo: {
      return "XCIS.BBO";
    }
    case Dataset::XcisTrades: {
      return "XCIS.TRADES";
    }
    case Dataset::MemxMemoir: {
      return "MEMX.MEMOIR";
    }
    case Dataset::EprlDom: {
      return "EPRL.DOM";
    }
    case Dataset::FinnNls: {
      return "FINN.NLS";
    }
    case Dataset::FinyTrades: {
      return "FINY.TRADES";
    }
    case Dataset::OpraPillar: {
      return "OPRA.PILLAR";
    }
    case Dataset::DbeqBasic: {
      return "DBEQ.BASIC";
    }
    case Dataset::ArcxPillar: {
      return "ARCX.PILLAR";
    }
    case Dataset::IexgTops: {
      return "IEXG.TOPS";
    }
    case Dataset::EqusPlus: {
      return "EQUS.PLUS";
    }
    case Dataset::XnysBbo: {
      return "XNYS.BBO";
    }
    case Dataset::XnysTrades: {
      return "XNYS.TRADES";
    }
    case Dataset::XnasQbbo: {
      return "XNAS.QBBO";
    }
    case Dataset::XnasNls: {
      return "XNAS.NLS";
    }
    case Dataset::IfeuImpact: {
      return "IFEU.IMPACT";
    }
    case Dataset::NdexImpact: {
      return "NDEX.IMPACT";
    }
    case Dataset::EqusAll: {
      return "EQUS.ALL";
    }
    case Dataset::XnasBasic: {
      return "XNAS.BASIC";
    }
    case Dataset::EqusSummary: {
      return "EQUS.SUMMARY";
    }
    case Dataset::XcisTradesbbo: {
      return "XCIS.TRADESBBO";
    }
    case Dataset::XnysTradesbbo: {
      return "XNYS.TRADESBBO";
    }
    case Dataset::EqusMini: {
      return "EQUS.MINI";
    }
    case Dataset::IfusImpact: {
      return "IFUS.IMPACT";
    }
    case Dataset::IfllImpact: {
      return "IFLL.IMPACT";
    }
    case Dataset::XeurEobi: {
      return "XEUR.EOBI";
    }
    case Dataset::XeeeEobi: {
      return "XEEE.EOBI";
    }
    default: {
      return "Unknown";
    }
  }
}

std::ostream& operator<<(std::ostream& out, Dataset dataset) {
  out << ToString(dataset);
  return out;
}

template <>
Dataset FromString(const std::string& str) {
  if (str == "GLBX.MDP3") {
    return Dataset::GlbxMdp3;
  }
  if (str == "XNAS.ITCH") {
    return Dataset::XnasItch;
  }
  if (str == "XBOS.ITCH") {
    return Dataset::XbosItch;
  }
  if (str == "XPSX.ITCH") {
    return Dataset::XpsxItch;
  }
  if (str == "BATS.PITCH") {
    return Dataset::BatsPitch;
  }
  if (str == "BATY.PITCH") {
    return Dataset::BatyPitch;
  }
  if (str == "EDGA.PITCH") {
    return Dataset::EdgaPitch;
  }
  if (str == "EDGX.PITCH") {
    return Dataset::EdgxPitch;
  }
  if (str == "XNYS.PILLAR") {
    return Dataset::XnysPillar;
  }
  if (str == "XCIS.PILLAR") {
    return Dataset::XcisPillar;
  }
  if (str == "XASE.PILLAR") {
    return Dataset::XasePillar;
  }
  if (str == "XCHI.PILLAR") {
    return Dataset::XchiPillar;
  }
  if (str == "XCIS.BBO") {
    return Dataset::XcisBbo;
  }
  if (str == "XCIS.TRADES") {
    return Dataset::XcisTrades;
  }
  if (str == "MEMX.MEMOIR") {
    return Dataset::MemxMemoir;
  }
  if (str == "EPRL.DOM") {
    return Dataset::EprlDom;
  }
  if (str == "FINN.NLS") {
    return Dataset::FinnNls;
  }
  if (str == "FINY.TRADES") {
    return Dataset::FinyTrades;
  }
  if (str == "OPRA.PILLAR") {
    return Dataset::OpraPillar;
  }
  if (str == "DBEQ.BASIC") {
    return Dataset::DbeqBasic;
  }
  if (str == "ARCX.PILLAR") {
    return Dataset::ArcxPillar;
  }
  if (str == "IEXG.TOPS") {
    return Dataset::IexgTops;
  }
  if (str == "EQUS.PLUS") {
    return Dataset::EqusPlus;
  }
  if (str == "XNYS.BBO") {
    return Dataset::XnysBbo;
  }
  if (str == "XNYS.TRADES") {
    return Dataset::XnysTrades;
  }
  if (str == "XNAS.QBBO") {
    return Dataset::XnasQbbo;
  }
  if (str == "XNAS.NLS") {
    return Dataset::XnasNls;
  }
  if (str == "IFEU.IMPACT") {
    return Dataset::IfeuImpact;
  }
  if (str == "NDEX.IMPACT") {
    return Dataset::NdexImpact;
  }
  if (str == "EQUS.ALL") {
    return Dataset::EqusAll;
  }
  if (str == "XNAS.BASIC") {
    return Dataset::XnasBasic;
  }
  if (str == "EQUS.SUMMARY") {
    return Dataset::EqusSummary;
  }
  if (str == "XCIS.TRADESBBO") {
    return Dataset::XcisTradesbbo;
  }
  if (str == "XNYS.TRADESBBO") {
    return Dataset::XnysTradesbbo;
  }
  if (str == "EQUS.MINI") {
    return Dataset::EqusMini;
  }
  if (str == "IFUS.IMPACT") {
    return Dataset::IfusImpact;
  }
  if (str == "IFLL.IMPACT") {
    return Dataset::IfllImpact;
  }
  if (str == "XEUR.EOBI") {
    return Dataset::XeurEobi;
  }
  if (str == "XEEE.EOBI") {
    return Dataset::XeeeEobi;
  }
  throw InvalidArgumentError{"FromString<Dataset>", "str",
                             "unknown value '" + str + '\''};
}

Venue PublisherVenue(Publisher publisher) {
  switch (publisher) {
    case Publisher::GlbxMdp3Glbx: {
      return Venue::Glbx;
    }
    case Publisher::XnasItchXnas: {
      return Venue::Xnas;
    }
    case Publisher::XbosItchXbos: {
      return Venue::Xbos;
    }
    case Publisher::XpsxItchXpsx: {
      return Venue::Xpsx;
    }
    case Publisher::BatsPitchBats: {
      return Venue::Bats;
    }
    case Publisher::BatyPitchBaty: {
      return Venue::Baty;
    }
    case Publisher::EdgaPitchEdga: {
      return Venue::Edga;
    }
    case Publisher::EdgxPitchEdgx: {
      return Venue::Edgx;
    }
    case Publisher::XnysPillarXnys: {
      return Venue::Xnys;
    }
    case Publisher::XcisPillarXcis: {
      return Venue::Xcis;
    }
    case Publisher::XasePillarXase: {
      return Venue::Xase;
    }
    case Publisher::XchiPillarXchi: {
      return Venue::Xchi;
    }
    case Publisher::XcisBboXcis: {
      return Venue::Xcis;
    }
    case Publisher::XcisTradesXcis: {
      return Venue::Xcis;
    }
    case Publisher::MemxMemoirMemx: {
      return Venue::Memx;
    }
    case Publisher::EprlDomEprl: {
      return Venue::Eprl;
    }
    case Publisher::XnasNlsFinn: {
      return Venue::Finn;
    }
    case Publisher::XnasNlsFinc: {
      return Venue::Finc;
    }
    case Publisher::XnysTradesFiny: {
      return Venue::Finy;
    }
    case Publisher::OpraPillarAmxo: {
      return Venue::Amxo;
    }
    case Publisher::OpraPillarXbox: {
      return Venue::Xbox;
    }
    case Publisher::OpraPillarXcbo: {
      return Venue::Xcbo;
    }
    case Publisher::OpraPillarEmld: {
      return Venue::Emld;
    }
    case Publisher::OpraPillarEdgo: {
      return Venue::Edgo;
    }
    case Publisher::OpraPillarGmni: {
      return Venue::Gmni;
    }
    case Publisher::OpraPillarXisx: {
      return Venue::Xisx;
    }
    case Publisher::OpraPillarMcry: {
      return Venue::Mcry;
    }
    case Publisher::OpraPillarXmio: {
      return Venue::Xmio;
    }
    case Publisher::OpraPillarArco: {
      return Venue::Arco;
    }
    case Publisher::OpraPillarOpra: {
      return Venue::Opra;
    }
    case Publisher::OpraPillarMprl: {
      return Venue::Mprl;
    }
    case Publisher::OpraPillarXndq: {
      return Venue::Xndq;
    }
    case Publisher::OpraPillarXbxo: {
      return Venue::Xbxo;
    }
    case Publisher::OpraPillarC2Ox: {
      return Venue::C2Ox;
    }
    case Publisher::OpraPillarXphl: {
      return Venue::Xphl;
    }
    case Publisher::OpraPillarBato: {
      return Venue::Bato;
    }
    case Publisher::OpraPillarMxop: {
      return Venue::Mxop;
    }
    case Publisher::IexgTopsIexg: {
      return Venue::Iexg;
    }
    case Publisher::DbeqBasicXchi: {
      return Venue::Xchi;
    }
    case Publisher::DbeqBasicXcis: {
      return Venue::Xcis;
    }
    case Publisher::DbeqBasicIexg: {
      return Venue::Iexg;
    }
    case Publisher::DbeqBasicEprl: {
      return Venue::Eprl;
    }
    case Publisher::ArcxPillarArcx: {
      return Venue::Arcx;
    }
    case Publisher::XnysBboXnys: {
      return Venue::Xnys;
    }
    case Publisher::XnysTradesXnys: {
      return Venue::Xnys;
    }
    case Publisher::XnasQbboXnas: {
      return Venue::Xnas;
    }
    case Publisher::XnasNlsXnas: {
      return Venue::Xnas;
    }
    case Publisher::EqusPlusXchi: {
      return Venue::Xchi;
    }
    case Publisher::EqusPlusXcis: {
      return Venue::Xcis;
    }
    case Publisher::EqusPlusIexg: {
      return Venue::Iexg;
    }
    case Publisher::EqusPlusEprl: {
      return Venue::Eprl;
    }
    case Publisher::EqusPlusXnas: {
      return Venue::Xnas;
    }
    case Publisher::EqusPlusXnys: {
      return Venue::Xnys;
    }
    case Publisher::EqusPlusFinn: {
      return Venue::Finn;
    }
    case Publisher::EqusPlusFiny: {
      return Venue::Finy;
    }
    case Publisher::EqusPlusFinc: {
      return Venue::Finc;
    }
    case Publisher::IfeuImpactIfeu: {
      return Venue::Ifeu;
    }
    case Publisher::NdexImpactNdex: {
      return Venue::Ndex;
    }
    case Publisher::DbeqBasicDbeq: {
      return Venue::Dbeq;
    }
    case Publisher::EqusPlusEqus: {
      return Venue::Equs;
    }
    case Publisher::OpraPillarSphr: {
      return Venue::Sphr;
    }
    case Publisher::EqusAllXchi: {
      return Venue::Xchi;
    }
    case Publisher::EqusAllXcis: {
      return Venue::Xcis;
    }
    case Publisher::EqusAllIexg: {
      return Venue::Iexg;
    }
    case Publisher::EqusAllEprl: {
      return Venue::Eprl;
    }
    case Publisher::EqusAllXnas: {
      return Venue::Xnas;
    }
    case Publisher::EqusAllXnys: {
      return Venue::Xnys;
    }
    case Publisher::EqusAllFinn: {
      return Venue::Finn;
    }
    case Publisher::EqusAllFiny: {
      return Venue::Finy;
    }
    case Publisher::EqusAllFinc: {
      return Venue::Finc;
    }
    case Publisher::EqusAllBats: {
      return Venue::Bats;
    }
    case Publisher::EqusAllBaty: {
      return Venue::Baty;
    }
    case Publisher::EqusAllEdga: {
      return Venue::Edga;
    }
    case Publisher::EqusAllEdgx: {
      return Venue::Edgx;
    }
    case Publisher::EqusAllXbos: {
      return Venue::Xbos;
    }
    case Publisher::EqusAllXpsx: {
      return Venue::Xpsx;
    }
    case Publisher::EqusAllMemx: {
      return Venue::Memx;
    }
    case Publisher::EqusAllXase: {
      return Venue::Xase;
    }
    case Publisher::EqusAllArcx: {
      return Venue::Arcx;
    }
    case Publisher::EqusAllLtse: {
      return Venue::Ltse;
    }
    case Publisher::XnasBasicXnas: {
      return Venue::Xnas;
    }
    case Publisher::XnasBasicFinn: {
      return Venue::Finn;
    }
    case Publisher::XnasBasicFinc: {
      return Venue::Finc;
    }
    case Publisher::IfeuImpactXoff: {
      return Venue::Xoff;
    }
    case Publisher::NdexImpactXoff: {
      return Venue::Xoff;
    }
    case Publisher::XnasNlsXbos: {
      return Venue::Xbos;
    }
    case Publisher::XnasNlsXpsx: {
      return Venue::Xpsx;
    }
    case Publisher::XnasBasicXbos: {
      return Venue::Xbos;
    }
    case Publisher::XnasBasicXpsx: {
      return Venue::Xpsx;
    }
    case Publisher::EqusSummaryEqus: {
      return Venue::Equs;
    }
    case Publisher::XcisTradesbboXcis: {
      return Venue::Xcis;
    }
    case Publisher::XnysTradesbboXnys: {
      return Venue::Xnys;
    }
    case Publisher::XnasBasicEqus: {
      return Venue::Equs;
    }
    case Publisher::EqusAllEqus: {
      return Venue::Equs;
    }
    case Publisher::EqusMiniEqus: {
      return Venue::Equs;
    }
    case Publisher::XnysTradesEqus: {
      return Venue::Equs;
    }
    case Publisher::IfusImpactIfus: {
      return Venue::Ifus;
    }
    case Publisher::IfusImpactXoff: {
      return Venue::Xoff;
    }
    case Publisher::IfllImpactIfll: {
      return Venue::Ifll;
    }
    case Publisher::IfllImpactXoff: {
      return Venue::Xoff;
    }
    case Publisher::XeurEobiXeur: {
      return Venue::Xeur;
    }
    case Publisher::XeeeEobiXeee: {
      return Venue::Xeee;
    }
    case Publisher::XeurEobiXoff: {
      return Venue::Xoff;
    }
    case Publisher::XeeeEobiXoff: {
      return Venue::Xoff;
    }
    default: {
      throw InvalidArgumentError{
          "PublisherVenue", "publisher",
          "unknown conversion for " +
              std::to_string(static_cast<std::uint16_t>(publisher))};
    }
  }
}
Dataset PublisherDataset(Publisher publisher) {
  switch (publisher) {
    case Publisher::GlbxMdp3Glbx: {
      return Dataset::GlbxMdp3;
    }
    case Publisher::XnasItchXnas: {
      return Dataset::XnasItch;
    }
    case Publisher::XbosItchXbos: {
      return Dataset::XbosItch;
    }
    case Publisher::XpsxItchXpsx: {
      return Dataset::XpsxItch;
    }
    case Publisher::BatsPitchBats: {
      return Dataset::BatsPitch;
    }
    case Publisher::BatyPitchBaty: {
      return Dataset::BatyPitch;
    }
    case Publisher::EdgaPitchEdga: {
      return Dataset::EdgaPitch;
    }
    case Publisher::EdgxPitchEdgx: {
      return Dataset::EdgxPitch;
    }
    case Publisher::XnysPillarXnys: {
      return Dataset::XnysPillar;
    }
    case Publisher::XcisPillarXcis: {
      return Dataset::XcisPillar;
    }
    case Publisher::XasePillarXase: {
      return Dataset::XasePillar;
    }
    case Publisher::XchiPillarXchi: {
      return Dataset::XchiPillar;
    }
    case Publisher::XcisBboXcis: {
      return Dataset::XcisBbo;
    }
    case Publisher::XcisTradesXcis: {
      return Dataset::XcisTrades;
    }
    case Publisher::MemxMemoirMemx: {
      return Dataset::MemxMemoir;
    }
    case Publisher::EprlDomEprl: {
      return Dataset::EprlDom;
    }
    case Publisher::XnasNlsFinn: {
      return Dataset::XnasNls;
    }
    case Publisher::XnasNlsFinc: {
      return Dataset::XnasNls;
    }
    case Publisher::XnysTradesFiny: {
      return Dataset::XnysTrades;
    }
    case Publisher::OpraPillarAmxo: {
      return Dataset::OpraPillar;
    }
    case Publisher::OpraPillarXbox: {
      return Dataset::OpraPillar;
    }
    case Publisher::OpraPillarXcbo: {
      return Dataset::OpraPillar;
    }
    case Publisher::OpraPillarEmld: {
      return Dataset::OpraPillar;
    }
    case Publisher::OpraPillarEdgo: {
      return Dataset::OpraPillar;
    }
    case Publisher::OpraPillarGmni: {
      return Dataset::OpraPillar;
    }
    case Publisher::OpraPillarXisx: {
      return Dataset::OpraPillar;
    }
    case Publisher::OpraPillarMcry: {
      return Dataset::OpraPillar;
    }
    case Publisher::OpraPillarXmio: {
      return Dataset::OpraPillar;
    }
    case Publisher::OpraPillarArco: {
      return Dataset::OpraPillar;
    }
    case Publisher::OpraPillarOpra: {
      return Dataset::OpraPillar;
    }
    case Publisher::OpraPillarMprl: {
      return Dataset::OpraPillar;
    }
    case Publisher::OpraPillarXndq: {
      return Dataset::OpraPillar;
    }
    case Publisher::OpraPillarXbxo: {
      return Dataset::OpraPillar;
    }
    case Publisher::OpraPillarC2Ox: {
      return Dataset::OpraPillar;
    }
    case Publisher::OpraPillarXphl: {
      return Dataset::OpraPillar;
    }
    case Publisher::OpraPillarBato: {
      return Dataset::OpraPillar;
    }
    case Publisher::OpraPillarMxop: {
      return Dataset::OpraPillar;
    }
    case Publisher::IexgTopsIexg: {
      return Dataset::IexgTops;
    }
    case Publisher::DbeqBasicXchi: {
      return Dataset::DbeqBasic;
    }
    case Publisher::DbeqBasicXcis: {
      return Dataset::DbeqBasic;
    }
    case Publisher::DbeqBasicIexg: {
      return Dataset::DbeqBasic;
    }
    case Publisher::DbeqBasicEprl: {
      return Dataset::DbeqBasic;
    }
    case Publisher::ArcxPillarArcx: {
      return Dataset::ArcxPillar;
    }
    case Publisher::XnysBboXnys: {
      return Dataset::XnysBbo;
    }
    case Publisher::XnysTradesXnys: {
      return Dataset::XnysTrades;
    }
    case Publisher::XnasQbboXnas: {
      return Dataset::XnasQbbo;
    }
    case Publisher::XnasNlsXnas: {
      return Dataset::XnasNls;
    }
    case Publisher::EqusPlusXchi: {
      return Dataset::EqusPlus;
    }
    case Publisher::EqusPlusXcis: {
      return Dataset::EqusPlus;
    }
    case Publisher::EqusPlusIexg: {
      return Dataset::EqusPlus;
    }
    case Publisher::EqusPlusEprl: {
      return Dataset::EqusPlus;
    }
    case Publisher::EqusPlusXnas: {
      return Dataset::EqusPlus;
    }
    case Publisher::EqusPlusXnys: {
      return Dataset::EqusPlus;
    }
    case Publisher::EqusPlusFinn: {
      return Dataset::EqusPlus;
    }
    case Publisher::EqusPlusFiny: {
      return Dataset::EqusPlus;
    }
    case Publisher::EqusPlusFinc: {
      return Dataset::EqusPlus;
    }
    case Publisher::IfeuImpactIfeu: {
      return Dataset::IfeuImpact;
    }
    case Publisher::NdexImpactNdex: {
      return Dataset::NdexImpact;
    }
    case Publisher::DbeqBasicDbeq: {
      return Dataset::DbeqBasic;
    }
    case Publisher::EqusPlusEqus: {
      return Dataset::EqusPlus;
    }
    case Publisher::OpraPillarSphr: {
      return Dataset::OpraPillar;
    }
    case Publisher::EqusAllXchi: {
      return Dataset::EqusAll;
    }
    case Publisher::EqusAllXcis: {
      return Dataset::EqusAll;
    }
    case Publisher::EqusAllIexg: {
      return Dataset::EqusAll;
    }
    case Publisher::EqusAllEprl: {
      return Dataset::EqusAll;
    }
    case Publisher::EqusAllXnas: {
      return Dataset::EqusAll;
    }
    case Publisher::EqusAllXnys: {
      return Dataset::EqusAll;
    }
    case Publisher::EqusAllFinn: {
      return Dataset::EqusAll;
    }
    case Publisher::EqusAllFiny: {
      return Dataset::EqusAll;
    }
    case Publisher::EqusAllFinc: {
      return Dataset::EqusAll;
    }
    case Publisher::EqusAllBats: {
      return Dataset::EqusAll;
    }
    case Publisher::EqusAllBaty: {
      return Dataset::EqusAll;
    }
    case Publisher::EqusAllEdga: {
      return Dataset::EqusAll;
    }
    case Publisher::EqusAllEdgx: {
      return Dataset::EqusAll;
    }
    case Publisher::EqusAllXbos: {
      return Dataset::EqusAll;
    }
    case Publisher::EqusAllXpsx: {
      return Dataset::EqusAll;
    }
    case Publisher::EqusAllMemx: {
      return Dataset::EqusAll;
    }
    case Publisher::EqusAllXase: {
      return Dataset::EqusAll;
    }
    case Publisher::EqusAllArcx: {
      return Dataset::EqusAll;
    }
    case Publisher::EqusAllLtse: {
      return Dataset::EqusAll;
    }
    case Publisher::XnasBasicXnas: {
      return Dataset::XnasBasic;
    }
    case Publisher::XnasBasicFinn: {
      return Dataset::XnasBasic;
    }
    case Publisher::XnasBasicFinc: {
      return Dataset::XnasBasic;
    }
    case Publisher::IfeuImpactXoff: {
      return Dataset::IfeuImpact;
    }
    case Publisher::NdexImpactXoff: {
      return Dataset::NdexImpact;
    }
    case Publisher::XnasNlsXbos: {
      return Dataset::XnasNls;
    }
    case Publisher::XnasNlsXpsx: {
      return Dataset::XnasNls;
    }
    case Publisher::XnasBasicXbos: {
      return Dataset::XnasBasic;
    }
    case Publisher::XnasBasicXpsx: {
      return Dataset::XnasBasic;
    }
    case Publisher::EqusSummaryEqus: {
      return Dataset::EqusSummary;
    }
    case Publisher::XcisTradesbboXcis: {
      return Dataset::XcisTradesbbo;
    }
    case Publisher::XnysTradesbboXnys: {
      return Dataset::XnysTradesbbo;
    }
    case Publisher::XnasBasicEqus: {
      return Dataset::XnasBasic;
    }
    case Publisher::EqusAllEqus: {
      return Dataset::EqusAll;
    }
    case Publisher::EqusMiniEqus: {
      return Dataset::EqusMini;
    }
    case Publisher::XnysTradesEqus: {
      return Dataset::XnysTrades;
    }
    case Publisher::IfusImpactIfus: {
      return Dataset::IfusImpact;
    }
    case Publisher::IfusImpactXoff: {
      return Dataset::IfusImpact;
    }
    case Publisher::IfllImpactIfll: {
      return Dataset::IfllImpact;
    }
    case Publisher::IfllImpactXoff: {
      return Dataset::IfllImpact;
    }
    case Publisher::XeurEobiXeur: {
      return Dataset::XeurEobi;
    }
    case Publisher::XeeeEobiXeee: {
      return Dataset::XeeeEobi;
    }
    case Publisher::XeurEobiXoff: {
      return Dataset::XeurEobi;
    }
    case Publisher::XeeeEobiXoff: {
      return Dataset::XeeeEobi;
    }
    default: {
      throw InvalidArgumentError{
          "PublisherDataset", "publisher",
          "unknown conversion for " +
              std::to_string(static_cast<std::uint16_t>(publisher))};
    }
  }
}

const char* ToString(Publisher publisher) {
  switch (publisher) {
    case Publisher::GlbxMdp3Glbx: {
      return "GLBX.MDP3.GLBX";
    }
    case Publisher::XnasItchXnas: {
      return "XNAS.ITCH.XNAS";
    }
    case Publisher::XbosItchXbos: {
      return "XBOS.ITCH.XBOS";
    }
    case Publisher::XpsxItchXpsx: {
      return "XPSX.ITCH.XPSX";
    }
    case Publisher::BatsPitchBats: {
      return "BATS.PITCH.BATS";
    }
    case Publisher::BatyPitchBaty: {
      return "BATY.PITCH.BATY";
    }
    case Publisher::EdgaPitchEdga: {
      return "EDGA.PITCH.EDGA";
    }
    case Publisher::EdgxPitchEdgx: {
      return "EDGX.PITCH.EDGX";
    }
    case Publisher::XnysPillarXnys: {
      return "XNYS.PILLAR.XNYS";
    }
    case Publisher::XcisPillarXcis: {
      return "XCIS.PILLAR.XCIS";
    }
    case Publisher::XasePillarXase: {
      return "XASE.PILLAR.XASE";
    }
    case Publisher::XchiPillarXchi: {
      return "XCHI.PILLAR.XCHI";
    }
    case Publisher::XcisBboXcis: {
      return "XCIS.BBO.XCIS";
    }
    case Publisher::XcisTradesXcis: {
      return "XCIS.TRADES.XCIS";
    }
    case Publisher::MemxMemoirMemx: {
      return "MEMX.MEMOIR.MEMX";
    }
    case Publisher::EprlDomEprl: {
      return "EPRL.DOM.EPRL";
    }
    case Publisher::XnasNlsFinn: {
      return "XNAS.NLS.FINN";
    }
    case Publisher::XnasNlsFinc: {
      return "XNAS.NLS.FINC";
    }
    case Publisher::XnysTradesFiny: {
      return "XNYS.TRADES.FINY";
    }
    case Publisher::OpraPillarAmxo: {
      return "OPRA.PILLAR.AMXO";
    }
    case Publisher::OpraPillarXbox: {
      return "OPRA.PILLAR.XBOX";
    }
    case Publisher::OpraPillarXcbo: {
      return "OPRA.PILLAR.XCBO";
    }
    case Publisher::OpraPillarEmld: {
      return "OPRA.PILLAR.EMLD";
    }
    case Publisher::OpraPillarEdgo: {
      return "OPRA.PILLAR.EDGO";
    }
    case Publisher::OpraPillarGmni: {
      return "OPRA.PILLAR.GMNI";
    }
    case Publisher::OpraPillarXisx: {
      return "OPRA.PILLAR.XISX";
    }
    case Publisher::OpraPillarMcry: {
      return "OPRA.PILLAR.MCRY";
    }
    case Publisher::OpraPillarXmio: {
      return "OPRA.PILLAR.XMIO";
    }
    case Publisher::OpraPillarArco: {
      return "OPRA.PILLAR.ARCO";
    }
    case Publisher::OpraPillarOpra: {
      return "OPRA.PILLAR.OPRA";
    }
    case Publisher::OpraPillarMprl: {
      return "OPRA.PILLAR.MPRL";
    }
    case Publisher::OpraPillarXndq: {
      return "OPRA.PILLAR.XNDQ";
    }
    case Publisher::OpraPillarXbxo: {
      return "OPRA.PILLAR.XBXO";
    }
    case Publisher::OpraPillarC2Ox: {
      return "OPRA.PILLAR.C2OX";
    }
    case Publisher::OpraPillarXphl: {
      return "OPRA.PILLAR.XPHL";
    }
    case Publisher::OpraPillarBato: {
      return "OPRA.PILLAR.BATO";
    }
    case Publisher::OpraPillarMxop: {
      return "OPRA.PILLAR.MXOP";
    }
    case Publisher::IexgTopsIexg: {
      return "IEXG.TOPS.IEXG";
    }
    case Publisher::DbeqBasicXchi: {
      return "DBEQ.BASIC.XCHI";
    }
    case Publisher::DbeqBasicXcis: {
      return "DBEQ.BASIC.XCIS";
    }
    case Publisher::DbeqBasicIexg: {
      return "DBEQ.BASIC.IEXG";
    }
    case Publisher::DbeqBasicEprl: {
      return "DBEQ.BASIC.EPRL";
    }
    case Publisher::ArcxPillarArcx: {
      return "ARCX.PILLAR.ARCX";
    }
    case Publisher::XnysBboXnys: {
      return "XNYS.BBO.XNYS";
    }
    case Publisher::XnysTradesXnys: {
      return "XNYS.TRADES.XNYS";
    }
    case Publisher::XnasQbboXnas: {
      return "XNAS.QBBO.XNAS";
    }
    case Publisher::XnasNlsXnas: {
      return "XNAS.NLS.XNAS";
    }
    case Publisher::EqusPlusXchi: {
      return "EQUS.PLUS.XCHI";
    }
    case Publisher::EqusPlusXcis: {
      return "EQUS.PLUS.XCIS";
    }
    case Publisher::EqusPlusIexg: {
      return "EQUS.PLUS.IEXG";
    }
    case Publisher::EqusPlusEprl: {
      return "EQUS.PLUS.EPRL";
    }
    case Publisher::EqusPlusXnas: {
      return "EQUS.PLUS.XNAS";
    }
    case Publisher::EqusPlusXnys: {
      return "EQUS.PLUS.XNYS";
    }
    case Publisher::EqusPlusFinn: {
      return "EQUS.PLUS.FINN";
    }
    case Publisher::EqusPlusFiny: {
      return "EQUS.PLUS.FINY";
    }
    case Publisher::EqusPlusFinc: {
      return "EQUS.PLUS.FINC";
    }
    case Publisher::IfeuImpactIfeu: {
      return "IFEU.IMPACT.IFEU";
    }
    case Publisher::NdexImpactNdex: {
      return "NDEX.IMPACT.NDEX";
    }
    case Publisher::DbeqBasicDbeq: {
      return "DBEQ.BASIC.DBEQ";
    }
    case Publisher::EqusPlusEqus: {
      return "EQUS.PLUS.EQUS";
    }
    case Publisher::OpraPillarSphr: {
      return "OPRA.PILLAR.SPHR";
    }
    case Publisher::EqusAllXchi: {
      return "EQUS.ALL.XCHI";
    }
    case Publisher::EqusAllXcis: {
      return "EQUS.ALL.XCIS";
    }
    case Publisher::EqusAllIexg: {
      return "EQUS.ALL.IEXG";
    }
    case Publisher::EqusAllEprl: {
      return "EQUS.ALL.EPRL";
    }
    case Publisher::EqusAllXnas: {
      return "EQUS.ALL.XNAS";
    }
    case Publisher::EqusAllXnys: {
      return "EQUS.ALL.XNYS";
    }
    case Publisher::EqusAllFinn: {
      return "EQUS.ALL.FINN";
    }
    case Publisher::EqusAllFiny: {
      return "EQUS.ALL.FINY";
    }
    case Publisher::EqusAllFinc: {
      return "EQUS.ALL.FINC";
    }
    case Publisher::EqusAllBats: {
      return "EQUS.ALL.BATS";
    }
    case Publisher::EqusAllBaty: {
      return "EQUS.ALL.BATY";
    }
    case Publisher::EqusAllEdga: {
      return "EQUS.ALL.EDGA";
    }
    case Publisher::EqusAllEdgx: {
      return "EQUS.ALL.EDGX";
    }
    case Publisher::EqusAllXbos: {
      return "EQUS.ALL.XBOS";
    }
    case Publisher::EqusAllXpsx: {
      return "EQUS.ALL.XPSX";
    }
    case Publisher::EqusAllMemx: {
      return "EQUS.ALL.MEMX";
    }
    case Publisher::EqusAllXase: {
      return "EQUS.ALL.XASE";
    }
    case Publisher::EqusAllArcx: {
      return "EQUS.ALL.ARCX";
    }
    case Publisher::EqusAllLtse: {
      return "EQUS.ALL.LTSE";
    }
    case Publisher::XnasBasicXnas: {
      return "XNAS.BASIC.XNAS";
    }
    case Publisher::XnasBasicFinn: {
      return "XNAS.BASIC.FINN";
    }
    case Publisher::XnasBasicFinc: {
      return "XNAS.BASIC.FINC";
    }
    case Publisher::IfeuImpactXoff: {
      return "IFEU.IMPACT.XOFF";
    }
    case Publisher::NdexImpactXoff: {
      return "NDEX.IMPACT.XOFF";
    }
    case Publisher::XnasNlsXbos: {
      return "XNAS.NLS.XBOS";
    }
    case Publisher::XnasNlsXpsx: {
      return "XNAS.NLS.XPSX";
    }
    case Publisher::XnasBasicXbos: {
      return "XNAS.BASIC.XBOS";
    }
    case Publisher::XnasBasicXpsx: {
      return "XNAS.BASIC.XPSX";
    }
    case Publisher::EqusSummaryEqus: {
      return "EQUS.SUMMARY.EQUS";
    }
    case Publisher::XcisTradesbboXcis: {
      return "XCIS.TRADESBBO.XCIS";
    }
    case Publisher::XnysTradesbboXnys: {
      return "XNYS.TRADESBBO.XNYS";
    }
    case Publisher::XnasBasicEqus: {
      return "XNAS.BASIC.EQUS";
    }
    case Publisher::EqusAllEqus: {
      return "EQUS.ALL.EQUS";
    }
    case Publisher::EqusMiniEqus: {
      return "EQUS.MINI.EQUS";
    }
    case Publisher::XnysTradesEqus: {
      return "XNYS.TRADES.EQUS";
    }
    case Publisher::IfusImpactIfus: {
      return "IFUS.IMPACT.IFUS";
    }
    case Publisher::IfusImpactXoff: {
      return "IFUS.IMPACT.XOFF";
    }
    case Publisher::IfllImpactIfll: {
      return "IFLL.IMPACT.IFLL";
    }
    case Publisher::IfllImpactXoff: {
      return "IFLL.IMPACT.XOFF";
    }
    case Publisher::XeurEobiXeur: {
      return "XEUR.EOBI.XEUR";
    }
    case Publisher::XeeeEobiXeee: {
      return "XEEE.EOBI.XEEE";
    }
    case Publisher::XeurEobiXoff: {
      return "XEUR.EOBI.XOFF";
    }
    case Publisher::XeeeEobiXoff: {
      return "XEEE.EOBI.XOFF";
    }
    default: {
      return "Unknown";
    }
  }
}

std::ostream& operator<<(std::ostream& out, Publisher publisher) {
  out << ToString(publisher);
  return out;
}

template <>
Publisher FromString(const std::string& str) {
  if (str == "GLBX.MDP3.GLBX") {
    return Publisher::GlbxMdp3Glbx;
  }
  if (str == "XNAS.ITCH.XNAS") {
    return Publisher::XnasItchXnas;
  }
  if (str == "XBOS.ITCH.XBOS") {
    return Publisher::XbosItchXbos;
  }
  if (str == "XPSX.ITCH.XPSX") {
    return Publisher::XpsxItchXpsx;
  }
  if (str == "BATS.PITCH.BATS") {
    return Publisher::BatsPitchBats;
  }
  if (str == "BATY.PITCH.BATY") {
    return Publisher::BatyPitchBaty;
  }
  if (str == "EDGA.PITCH.EDGA") {
    return Publisher::EdgaPitchEdga;
  }
  if (str == "EDGX.PITCH.EDGX") {
    return Publisher::EdgxPitchEdgx;
  }
  if (str == "XNYS.PILLAR.XNYS") {
    return Publisher::XnysPillarXnys;
  }
  if (str == "XCIS.PILLAR.XCIS") {
    return Publisher::XcisPillarXcis;
  }
  if (str == "XASE.PILLAR.XASE") {
    return Publisher::XasePillarXase;
  }
  if (str == "XCHI.PILLAR.XCHI") {
    return Publisher::XchiPillarXchi;
  }
  if (str == "XCIS.BBO.XCIS") {
    return Publisher::XcisBboXcis;
  }
  if (str == "XCIS.TRADES.XCIS") {
    return Publisher::XcisTradesXcis;
  }
  if (str == "MEMX.MEMOIR.MEMX") {
    return Publisher::MemxMemoirMemx;
  }
  if (str == "EPRL.DOM.EPRL") {
    return Publisher::EprlDomEprl;
  }
  if (str == "XNAS.NLS.FINN") {
    return Publisher::XnasNlsFinn;
  }
  if (str == "XNAS.NLS.FINC") {
    return Publisher::XnasNlsFinc;
  }
  if (str == "XNYS.TRADES.FINY") {
    return Publisher::XnysTradesFiny;
  }
  if (str == "OPRA.PILLAR.AMXO") {
    return Publisher::OpraPillarAmxo;
  }
  if (str == "OPRA.PILLAR.XBOX") {
    return Publisher::OpraPillarXbox;
  }
  if (str == "OPRA.PILLAR.XCBO") {
    return Publisher::OpraPillarXcbo;
  }
  if (str == "OPRA.PILLAR.EMLD") {
    return Publisher::OpraPillarEmld;
  }
  if (str == "OPRA.PILLAR.EDGO") {
    return Publisher::OpraPillarEdgo;
  }
  if (str == "OPRA.PILLAR.GMNI") {
    return Publisher::OpraPillarGmni;
  }
  if (str == "OPRA.PILLAR.XISX") {
    return Publisher::OpraPillarXisx;
  }
  if (str == "OPRA.PILLAR.MCRY") {
    return Publisher::OpraPillarMcry;
  }
  if (str == "OPRA.PILLAR.XMIO") {
    return Publisher::OpraPillarXmio;
  }
  if (str == "OPRA.PILLAR.ARCO") {
    return Publisher::OpraPillarArco;
  }
  if (str == "OPRA.PILLAR.OPRA") {
    return Publisher::OpraPillarOpra;
  }
  if (str == "OPRA.PILLAR.MPRL") {
    return Publisher::OpraPillarMprl;
  }
  if (str == "OPRA.PILLAR.XNDQ") {
    return Publisher::OpraPillarXndq;
  }
  if (str == "OPRA.PILLAR.XBXO") {
    return Publisher::OpraPillarXbxo;
  }
  if (str == "OPRA.PILLAR.C2OX") {
    return Publisher::OpraPillarC2Ox;
  }
  if (str == "OPRA.PILLAR.XPHL") {
    return Publisher::OpraPillarXphl;
  }
  if (str == "OPRA.PILLAR.BATO") {
    return Publisher::OpraPillarBato;
  }
  if (str == "OPRA.PILLAR.MXOP") {
    return Publisher::OpraPillarMxop;
  }
  if (str == "IEXG.TOPS.IEXG") {
    return Publisher::IexgTopsIexg;
  }
  if (str == "DBEQ.BASIC.XCHI") {
    return Publisher::DbeqBasicXchi;
  }
  if (str == "DBEQ.BASIC.XCIS") {
    return Publisher::DbeqBasicXcis;
  }
  if (str == "DBEQ.BASIC.IEXG") {
    return Publisher::DbeqBasicIexg;
  }
  if (str == "DBEQ.BASIC.EPRL") {
    return Publisher::DbeqBasicEprl;
  }
  if (str == "ARCX.PILLAR.ARCX") {
    return Publisher::ArcxPillarArcx;
  }
  if (str == "XNYS.BBO.XNYS") {
    return Publisher::XnysBboXnys;
  }
  if (str == "XNYS.TRADES.XNYS") {
    return Publisher::XnysTradesXnys;
  }
  if (str == "XNAS.QBBO.XNAS") {
    return Publisher::XnasQbboXnas;
  }
  if (str == "XNAS.NLS.XNAS") {
    return Publisher::XnasNlsXnas;
  }
  if (str == "EQUS.PLUS.XCHI") {
    return Publisher::EqusPlusXchi;
  }
  if (str == "EQUS.PLUS.XCIS") {
    return Publisher::EqusPlusXcis;
  }
  if (str == "EQUS.PLUS.IEXG") {
    return Publisher::EqusPlusIexg;
  }
  if (str == "EQUS.PLUS.EPRL") {
    return Publisher::EqusPlusEprl;
  }
  if (str == "EQUS.PLUS.XNAS") {
    return Publisher::EqusPlusXnas;
  }
  if (str == "EQUS.PLUS.XNYS") {
    return Publisher::EqusPlusXnys;
  }
  if (str == "EQUS.PLUS.FINN") {
    return Publisher::EqusPlusFinn;
  }
  if (str == "EQUS.PLUS.FINY") {
    return Publisher::EqusPlusFiny;
  }
  if (str == "EQUS.PLUS.FINC") {
    return Publisher::EqusPlusFinc;
  }
  if (str == "IFEU.IMPACT.IFEU") {
    return Publisher::IfeuImpactIfeu;
  }
  if (str == "NDEX.IMPACT.NDEX") {
    return Publisher::NdexImpactNdex;
  }
  if (str == "DBEQ.BASIC.DBEQ") {
    return Publisher::DbeqBasicDbeq;
  }
  if (str == "EQUS.PLUS.EQUS") {
    return Publisher::EqusPlusEqus;
  }
  if (str == "OPRA.PILLAR.SPHR") {
    return Publisher::OpraPillarSphr;
  }
  if (str == "EQUS.ALL.XCHI") {
    return Publisher::EqusAllXchi;
  }
  if (str == "EQUS.ALL.XCIS") {
    return Publisher::EqusAllXcis;
  }
  if (str == "EQUS.ALL.IEXG") {
    return Publisher::EqusAllIexg;
  }
  if (str == "EQUS.ALL.EPRL") {
    return Publisher::EqusAllEprl;
  }
  if (str == "EQUS.ALL.XNAS") {
    return Publisher::EqusAllXnas;
  }
  if (str == "EQUS.ALL.XNYS") {
    return Publisher::EqusAllXnys;
  }
  if (str == "EQUS.ALL.FINN") {
    return Publisher::EqusAllFinn;
  }
  if (str == "EQUS.ALL.FINY") {
    return Publisher::EqusAllFiny;
  }
  if (str == "EQUS.ALL.FINC") {
    return Publisher::EqusAllFinc;
  }
  if (str == "EQUS.ALL.BATS") {
    return Publisher::EqusAllBats;
  }
  if (str == "EQUS.ALL.BATY") {
    return Publisher::EqusAllBaty;
  }
  if (str == "EQUS.ALL.EDGA") {
    return Publisher::EqusAllEdga;
  }
  if (str == "EQUS.ALL.EDGX") {
    return Publisher::EqusAllEdgx;
  }
  if (str == "EQUS.ALL.XBOS") {
    return Publisher::EqusAllXbos;
  }
  if (str == "EQUS.ALL.XPSX") {
    return Publisher::EqusAllXpsx;
  }
  if (str == "EQUS.ALL.MEMX") {
    return Publisher::EqusAllMemx;
  }
  if (str == "EQUS.ALL.XASE") {
    return Publisher::EqusAllXase;
  }
  if (str == "EQUS.ALL.ARCX") {
    return Publisher::EqusAllArcx;
  }
  if (str == "EQUS.ALL.LTSE") {
    return Publisher::EqusAllLtse;
  }
  if (str == "XNAS.BASIC.XNAS") {
    return Publisher::XnasBasicXnas;
  }
  if (str == "XNAS.BASIC.FINN") {
    return Publisher::XnasBasicFinn;
  }
  if (str == "XNAS.BASIC.FINC") {
    return Publisher::XnasBasicFinc;
  }
  if (str == "IFEU.IMPACT.XOFF") {
    return Publisher::IfeuImpactXoff;
  }
  if (str == "NDEX.IMPACT.XOFF") {
    return Publisher::NdexImpactXoff;
  }
  if (str == "XNAS.NLS.XBOS") {
    return Publisher::XnasNlsXbos;
  }
  if (str == "XNAS.NLS.XPSX") {
    return Publisher::XnasNlsXpsx;
  }
  if (str == "XNAS.BASIC.XBOS") {
    return Publisher::XnasBasicXbos;
  }
  if (str == "XNAS.BASIC.XPSX") {
    return Publisher::XnasBasicXpsx;
  }
  if (str == "EQUS.SUMMARY.EQUS") {
    return Publisher::EqusSummaryEqus;
  }
  if (str == "XCIS.TRADESBBO.XCIS") {
    return Publisher::XcisTradesbboXcis;
  }
  if (str == "XNYS.TRADESBBO.XNYS") {
    return Publisher::XnysTradesbboXnys;
  }
  if (str == "XNAS.BASIC.EQUS") {
    return Publisher::XnasBasicEqus;
  }
  if (str == "EQUS.ALL.EQUS") {
    return Publisher::EqusAllEqus;
  }
  if (str == "EQUS.MINI.EQUS") {
    return Publisher::EqusMiniEqus;
  }
  if (str == "XNYS.TRADES.EQUS") {
    return Publisher::XnysTradesEqus;
  }
  if (str == "IFUS.IMPACT.IFUS") {
    return Publisher::IfusImpactIfus;
  }
  if (str == "IFUS.IMPACT.XOFF") {
    return Publisher::IfusImpactXoff;
  }
  if (str == "IFLL.IMPACT.IFLL") {
    return Publisher::IfllImpactIfll;
  }
  if (str == "IFLL.IMPACT.XOFF") {
    return Publisher::IfllImpactXoff;
  }
  if (str == "XEUR.EOBI.XEUR") {
    return Publisher::XeurEobiXeur;
  }
  if (str == "XEEE.EOBI.XEEE") {
    return Publisher::XeeeEobiXeee;
  }
  if (str == "XEUR.EOBI.XOFF") {
    return Publisher::XeurEobiXoff;
  }
  if (str == "XEEE.EOBI.XOFF") {
    return Publisher::XeeeEobiXoff;
  }
  throw InvalidArgumentError{"FromString<Publisher>", "str",
                             "unknown value '" + str + '\''};
}
// NOLINTEND(bugprone-branch-clone)
}  // namespace databento
