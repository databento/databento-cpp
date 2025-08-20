#include "databento/enums.hpp"

#include <cstdint>
#include <ostream>
#include <string>

#include "databento/exceptions.hpp"  // InvalidArgumentError

namespace databento {
const char* UrlFromGateway(HistoricalGateway gateway) {
  switch (gateway) {
    case HistoricalGateway::Bo1: {
      return "https://hist.databento.com";
    }
    default: {
      throw InvalidArgumentError{
          "UrlFromGateway", "gateway",
          "unknown value " + std::to_string(static_cast<std::uint8_t>(gateway))};
    }
  }
}

const char* ToString(FeedMode mode) {
  switch (mode) {
    case FeedMode::Historical: {
      return "historical";
    }
    case FeedMode::HistoricalStreaming: {
      return "historical-streaming";
    }
    case FeedMode::Live: {
      return "live";
    }
    default: {
      return "unknown";
    }
  }
}

const char* ToString(SplitDuration duration_interval) {
  switch (duration_interval) {
    case SplitDuration::Day: {
      return "day";
    }
    case SplitDuration::Week: {
      return "week";
    }
    case SplitDuration::Month: {
      return "month";
    }
    case SplitDuration::None: {
      return "none";
    }
    default: {
      return "unknown";
    }
  }
}

const char* ToString(Delivery delivery) {
  switch (delivery) {
    case Delivery::Download: {
      return "download";
    }
    default: {
      return "unknown";
    }
  }
}

const char* ToString(JobState state) {
  switch (state) {
    case JobState::Queued: {
      return "queued";
    }
    case JobState::Processing: {
      return "processing";
    }
    case JobState::Done: {
      return "done";
    }
    case JobState::Expired: {
      return "expired";
    }
    default: {
      return "unknown";
    }
  }
}

const char* ToString(DatasetCondition condition) {
  switch (condition) {
    case DatasetCondition::Available: {
      return "available";
    }
    case DatasetCondition::Degraded: {
      return "degraded";
    }
    case DatasetCondition::Pending: {
      return "pending";
    }
    case DatasetCondition::Missing: {
      return "missing";
    }
    default: {
      return "unknown";
    }
  }
}

const char* ToString(RType r_type) {
  switch (r_type) {
    case RType::Mbp0: {
      return "mbp-0";
    }
    case RType::Mbp1: {
      return "mbp-1";
    }
    case RType::Mbp10: {
      return "mbp-10";
    }
    case RType::Ohlcv1S: {
      return "ohlcv-1s";
    }
    case RType::Ohlcv1M: {
      return "ohlcv-1m";
    }
    case RType::Ohlcv1H: {
      return "ohlcv-1h";
    }
    case RType::Ohlcv1D: {
      return "ohlcv-1d";
    }
    case RType::OhlcvEod: {
      return "ohlcv-eod";
    }
    case RType::Status: {
      return "status";
    }
    case RType::InstrumentDef: {
      return "instrument-def";
    }
    case RType::Imbalance: {
      return "imbalance";
    }
    case RType::Error: {
      return "error";
    }
    case RType::SymbolMapping: {
      return "symbol-mapping";
    }
    case RType::System: {
      return "system";
    }
    case RType::Statistics: {
      return "statistics";
    }
    case RType::Mbo: {
      return "mbo";
    }
    case RType::Cmbp1: {
      return "cmbp-1";
    }
    case RType::Cbbo1S: {
      return "cbbo-1s";
    }
    case RType::Cbbo1M: {
      return "cbbo-1m";
    }
    case RType::Tcbbo: {
      return "tcbbo";
    }
    case RType::Bbo1S: {
      return "bbo-1s";
    }
    case RType::Bbo1M: {
      return "bbo-1m";
    }
    default: {
      return "Unknown";
    }
  }
}
const char* ToString(Side side) {
  switch (side) {
    case Side::Ask: {
      return "Ask";
    }
    case Side::Bid: {
      return "Bid";
    }
    case Side::None: {
      return "None";
    }
    default: {
      return "Unknown";
    }
  }
}
const char* ToString(Action action) {
  switch (action) {
    case Action::Modify: {
      return "Modify";
    }
    case Action::Trade: {
      return "Trade";
    }
    case Action::Fill: {
      return "Fill";
    }
    case Action::Cancel: {
      return "Cancel";
    }
    case Action::Add: {
      return "Add";
    }
    case Action::Clear: {
      return "Clear";
    }
    case Action::None: {
      return "None";
    }
    default: {
      return "Unknown";
    }
  }
}
const char* ToString(InstrumentClass instrument_class) {
  switch (instrument_class) {
    case InstrumentClass::Bond: {
      return "Bond";
    }
    case InstrumentClass::Call: {
      return "Call";
    }
    case InstrumentClass::Future: {
      return "Future";
    }
    case InstrumentClass::Stock: {
      return "Stock";
    }
    case InstrumentClass::MixedSpread: {
      return "MixedSpread";
    }
    case InstrumentClass::Put: {
      return "Put";
    }
    case InstrumentClass::FutureSpread: {
      return "FutureSpread";
    }
    case InstrumentClass::OptionSpread: {
      return "OptionSpread";
    }
    case InstrumentClass::FxSpot: {
      return "FxSpot";
    }
    case InstrumentClass::CommoditySpot: {
      return "CommoditySpot";
    }
    default: {
      return "Unknown";
    }
  }
}
const char* ToString(MatchAlgorithm match_algorithm) {
  switch (match_algorithm) {
    case MatchAlgorithm::Undefined: {
      return "Undefined";
    }
    case MatchAlgorithm::Fifo: {
      return "Fifo";
    }
    case MatchAlgorithm::Configurable: {
      return "Configurable";
    }
    case MatchAlgorithm::ProRata: {
      return "ProRata";
    }
    case MatchAlgorithm::FifoLmm: {
      return "FifoLmm";
    }
    case MatchAlgorithm::ThresholdProRata: {
      return "ThresholdProRata";
    }
    case MatchAlgorithm::FifoTopLmm: {
      return "FifoTopLmm";
    }
    case MatchAlgorithm::ThresholdProRataLmm: {
      return "ThresholdProRataLmm";
    }
    case MatchAlgorithm::EurodollarFutures: {
      return "EurodollarFutures";
    }
    case MatchAlgorithm::TimeProRata: {
      return "TimeProRata";
    }
    case MatchAlgorithm::InstitutionalPrioritization: {
      return "InstitutionalPrioritization";
    }
    default: {
      return "Unknown";
    }
  }
}
const char* ToString(UserDefinedInstrument user_defined_instrument) {
  switch (user_defined_instrument) {
    case UserDefinedInstrument::No: {
      return "No";
    }
    case UserDefinedInstrument::Yes: {
      return "Yes";
    }
    default: {
      return "Unknown";
    }
  }
}
const char* ToString(SecurityUpdateAction security_update_action) {
  switch (security_update_action) {
    case SecurityUpdateAction::Add: {
      return "Add";
    }
    case SecurityUpdateAction::Modify: {
      return "Modify";
    }
    case SecurityUpdateAction::Delete: {
      return "Delete";
    }
    default: {
      return "Unknown";
    }
  }
}
const char* ToString(SType s_type) {
  switch (s_type) {
    case SType::InstrumentId: {
      return "instrument_id";
    }
    case SType::RawSymbol: {
      return "raw_symbol";
    }
    case SType::Continuous: {
      return "continuous";
    }
    case SType::Parent: {
      return "parent";
    }
    case SType::NasdaqSymbol: {
      return "nasdaq_symbol";
    }
    case SType::CmsSymbol: {
      return "cms_symbol";
    }
    case SType::Isin: {
      return "isin";
    }
    case SType::UsCode: {
      return "us_code";
    }
    case SType::BbgCompId: {
      return "bbg_comp_id";
    }
    case SType::BbgCompTicker: {
      return "bbg_comp_ticker";
    }
    case SType::Figi: {
      return "figi";
    }
    case SType::FigiTicker: {
      return "figi_ticker";
    }
    default: {
      return "Unknown";
    }
  }
}
const char* ToString(Schema schema) {
  switch (schema) {
    case Schema::Mbo: {
      return "mbo";
    }
    case Schema::Mbp1: {
      return "mbp-1";
    }
    case Schema::Mbp10: {
      return "mbp-10";
    }
    case Schema::Tbbo: {
      return "tbbo";
    }
    case Schema::Trades: {
      return "trades";
    }
    case Schema::Ohlcv1S: {
      return "ohlcv-1s";
    }
    case Schema::Ohlcv1M: {
      return "ohlcv-1m";
    }
    case Schema::Ohlcv1H: {
      return "ohlcv-1h";
    }
    case Schema::Ohlcv1D: {
      return "ohlcv-1d";
    }
    case Schema::Definition: {
      return "definition";
    }
    case Schema::Statistics: {
      return "statistics";
    }
    case Schema::Status: {
      return "status";
    }
    case Schema::Imbalance: {
      return "imbalance";
    }
    case Schema::OhlcvEod: {
      return "ohlcv-eod";
    }
    case Schema::Cmbp1: {
      return "cmbp-1";
    }
    case Schema::Cbbo1S: {
      return "cbbo-1s";
    }
    case Schema::Cbbo1M: {
      return "cbbo-1m";
    }
    case Schema::Tcbbo: {
      return "tcbbo";
    }
    case Schema::Bbo1S: {
      return "bbo-1s";
    }
    case Schema::Bbo1M: {
      return "bbo-1m";
    }
    default: {
      return "Unknown";
    }
  }
}
const char* ToString(Encoding encoding) {
  switch (encoding) {
    case Encoding::Dbn: {
      return "dbn";
    }
    case Encoding::Csv: {
      return "csv";
    }
    case Encoding::Json: {
      return "json";
    }
    default: {
      return "Unknown";
    }
  }
}
const char* ToString(Compression compression) {
  switch (compression) {
    case Compression::None: {
      return "none";
    }
    case Compression::Zstd: {
      return "zstd";
    }
    default: {
      return "Unknown";
    }
  }
}
const char* ToString(StatType stat_type) {
  switch (stat_type) {
    case StatType::OpeningPrice: {
      return "OpeningPrice";
    }
    case StatType::IndicativeOpeningPrice: {
      return "IndicativeOpeningPrice";
    }
    case StatType::SettlementPrice: {
      return "SettlementPrice";
    }
    case StatType::TradingSessionLowPrice: {
      return "TradingSessionLowPrice";
    }
    case StatType::TradingSessionHighPrice: {
      return "TradingSessionHighPrice";
    }
    case StatType::ClearedVolume: {
      return "ClearedVolume";
    }
    case StatType::LowestOffer: {
      return "LowestOffer";
    }
    case StatType::HighestBid: {
      return "HighestBid";
    }
    case StatType::OpenInterest: {
      return "OpenInterest";
    }
    case StatType::FixingPrice: {
      return "FixingPrice";
    }
    case StatType::ClosePrice: {
      return "ClosePrice";
    }
    case StatType::NetChange: {
      return "NetChange";
    }
    case StatType::Vwap: {
      return "Vwap";
    }
    case StatType::Volatility: {
      return "Volatility";
    }
    case StatType::Delta: {
      return "Delta";
    }
    case StatType::UncrossingPrice: {
      return "UncrossingPrice";
    }
    default: {
      return "Unknown";
    }
  }
}
const char* ToString(StatUpdateAction stat_update_action) {
  switch (stat_update_action) {
    case StatUpdateAction::New: {
      return "New";
    }
    case StatUpdateAction::Delete: {
      return "Delete";
    }
    default: {
      return "Unknown";
    }
  }
}
const char* ToString(StatusAction status_action) {
  switch (status_action) {
    case StatusAction::None: {
      return "None";
    }
    case StatusAction::PreOpen: {
      return "PreOpen";
    }
    case StatusAction::PreCross: {
      return "PreCross";
    }
    case StatusAction::Quoting: {
      return "Quoting";
    }
    case StatusAction::Cross: {
      return "Cross";
    }
    case StatusAction::Rotation: {
      return "Rotation";
    }
    case StatusAction::NewPriceIndication: {
      return "NewPriceIndication";
    }
    case StatusAction::Trading: {
      return "Trading";
    }
    case StatusAction::Halt: {
      return "Halt";
    }
    case StatusAction::Pause: {
      return "Pause";
    }
    case StatusAction::Suspend: {
      return "Suspend";
    }
    case StatusAction::PreClose: {
      return "PreClose";
    }
    case StatusAction::Close: {
      return "Close";
    }
    case StatusAction::PostClose: {
      return "PostClose";
    }
    case StatusAction::SsrChange: {
      return "SsrChange";
    }
    case StatusAction::NotAvailableForTrading: {
      return "NotAvailableForTrading";
    }
    default: {
      return "Unknown";
    }
  }
}
const char* ToString(StatusReason status_reason) {
  switch (status_reason) {
    case StatusReason::None: {
      return "None";
    }
    case StatusReason::Scheduled: {
      return "Scheduled";
    }
    case StatusReason::SurveillanceIntervention: {
      return "SurveillanceIntervention";
    }
    case StatusReason::MarketEvent: {
      return "MarketEvent";
    }
    case StatusReason::InstrumentActivation: {
      return "InstrumentActivation";
    }
    case StatusReason::InstrumentExpiration: {
      return "InstrumentExpiration";
    }
    case StatusReason::RecoveryInProcess: {
      return "RecoveryInProcess";
    }
    case StatusReason::Regulatory: {
      return "Regulatory";
    }
    case StatusReason::Administrative: {
      return "Administrative";
    }
    case StatusReason::NonCompliance: {
      return "NonCompliance";
    }
    case StatusReason::FilingsNotCurrent: {
      return "FilingsNotCurrent";
    }
    case StatusReason::SecTradingSuspension: {
      return "SecTradingSuspension";
    }
    case StatusReason::NewIssue: {
      return "NewIssue";
    }
    case StatusReason::IssueAvailable: {
      return "IssueAvailable";
    }
    case StatusReason::IssuesReviewed: {
      return "IssuesReviewed";
    }
    case StatusReason::FilingReqsSatisfied: {
      return "FilingReqsSatisfied";
    }
    case StatusReason::NewsPending: {
      return "NewsPending";
    }
    case StatusReason::NewsReleased: {
      return "NewsReleased";
    }
    case StatusReason::NewsAndResumptionTimes: {
      return "NewsAndResumptionTimes";
    }
    case StatusReason::NewsNotForthcoming: {
      return "NewsNotForthcoming";
    }
    case StatusReason::OrderImbalance: {
      return "OrderImbalance";
    }
    case StatusReason::LuldPause: {
      return "LuldPause";
    }
    case StatusReason::Operational: {
      return "Operational";
    }
    case StatusReason::AdditionalInformationRequested: {
      return "AdditionalInformationRequested";
    }
    case StatusReason::MergerEffective: {
      return "MergerEffective";
    }
    case StatusReason::Etf: {
      return "Etf";
    }
    case StatusReason::CorporateAction: {
      return "CorporateAction";
    }
    case StatusReason::NewSecurityOffering: {
      return "NewSecurityOffering";
    }
    case StatusReason::MarketWideHaltLevel1: {
      return "MarketWideHaltLevel1";
    }
    case StatusReason::MarketWideHaltLevel2: {
      return "MarketWideHaltLevel2";
    }
    case StatusReason::MarketWideHaltLevel3: {
      return "MarketWideHaltLevel3";
    }
    case StatusReason::MarketWideHaltCarryover: {
      return "MarketWideHaltCarryover";
    }
    case StatusReason::MarketWideHaltResumption: {
      return "MarketWideHaltResumption";
    }
    case StatusReason::QuotationNotAvailable: {
      return "QuotationNotAvailable";
    }
    default: {
      return "Unknown";
    }
  }
}
const char* ToString(TradingEvent trading_event) {
  switch (trading_event) {
    case TradingEvent::None: {
      return "None";
    }
    case TradingEvent::NoCancel: {
      return "NoCancel";
    }
    case TradingEvent::ChangeTradingSession: {
      return "ChangeTradingSession";
    }
    case TradingEvent::ImpliedMatchingOn: {
      return "ImpliedMatchingOn";
    }
    case TradingEvent::ImpliedMatchingOff: {
      return "ImpliedMatchingOff";
    }
    default: {
      return "Unknown";
    }
  }
}
const char* ToString(TriState tri_state) {
  switch (tri_state) {
    case TriState::NotAvailable: {
      return "NotAvailable";
    }
    case TriState::No: {
      return "No";
    }
    case TriState::Yes: {
      return "Yes";
    }
    default: {
      return "Unknown";
    }
  }
}
const char* ToString(VersionUpgradePolicy version_upgrade_policy) {
  switch (version_upgrade_policy) {
    case VersionUpgradePolicy::AsIs: {
      return "AsIs";
    }
    case VersionUpgradePolicy::UpgradeToV2: {
      return "UpgradeToV2";
    }
    case VersionUpgradePolicy::UpgradeToV3: {
      return "UpgradeToV3";
    }
    default: {
      return "Unknown";
    }
  }
}
const char* ToString(ErrorCode error_code) {
  switch (error_code) {
    case ErrorCode::AuthFailed: {
      return "auth_failed";
    }
    case ErrorCode::ApiKeyDeactivated: {
      return "api_key_deactivated";
    }
    case ErrorCode::ConnectionLimitExceeded: {
      return "connection_limit_exceeded";
    }
    case ErrorCode::SymbolResolutionFailed: {
      return "symbol_resolution_failed";
    }
    case ErrorCode::InvalidSubscription: {
      return "invalid_subscription";
    }
    case ErrorCode::InternalError: {
      return "internal_error";
    }
    default: {
      return "Unknown";
    }
  }
}
const char* ToString(SystemCode system_code) {
  switch (system_code) {
    case SystemCode::Heartbeat: {
      return "heartbeat";
    }
    case SystemCode::SubscriptionAck: {
      return "subscription_ack";
    }
    case SystemCode::SlowReaderWarning: {
      return "slow_reader_warning";
    }
    case SystemCode::ReplayCompleted: {
      return "replay_completed";
    }
    case SystemCode::EndOfInterval: {
      return "end_of_interval";
    }
    default: {
      return "Unknown";
    }
  }
}

std::ostream& operator<<(std::ostream& out, FeedMode mode) {
  out << ToString(mode);
  return out;
}

std::ostream& operator<<(std::ostream& out, SplitDuration duration_interval) {
  out << ToString(duration_interval);
  return out;
}

std::ostream& operator<<(std::ostream& out, Delivery delivery) {
  out << ToString(delivery);
  return out;
}

std::ostream& operator<<(std::ostream& out, JobState state) {
  out << ToString(state);
  return out;
}

std::ostream& operator<<(std::ostream& out, DatasetCondition condition) {
  out << ToString(condition);
  return out;
}

std::ostream& operator<<(std::ostream& out, RType r_type) {
  out << ToString(r_type);
  return out;
}
std::ostream& operator<<(std::ostream& out, Side side) {
  out << ToString(side);
  return out;
}
std::ostream& operator<<(std::ostream& out, Action action) {
  out << ToString(action);
  return out;
}
std::ostream& operator<<(std::ostream& out, InstrumentClass instrument_class) {
  out << ToString(instrument_class);
  return out;
}
std::ostream& operator<<(std::ostream& out, MatchAlgorithm match_algorithm) {
  out << ToString(match_algorithm);
  return out;
}
std::ostream& operator<<(std::ostream& out,
                         UserDefinedInstrument user_defined_instrument) {
  out << ToString(user_defined_instrument);
  return out;
}
std::ostream& operator<<(std::ostream& out,
                         SecurityUpdateAction security_update_action) {
  out << ToString(security_update_action);
  return out;
}
std::ostream& operator<<(std::ostream& out, SType s_type) {
  out << ToString(s_type);
  return out;
}
std::ostream& operator<<(std::ostream& out, Schema schema) {
  out << ToString(schema);
  return out;
}
std::ostream& operator<<(std::ostream& out, Encoding encoding) {
  out << ToString(encoding);
  return out;
}
std::ostream& operator<<(std::ostream& out, Compression compression) {
  out << ToString(compression);
  return out;
}
std::ostream& operator<<(std::ostream& out, StatType stat_type) {
  out << ToString(stat_type);
  return out;
}
std::ostream& operator<<(std::ostream& out, StatUpdateAction stat_update_action) {
  out << ToString(stat_update_action);
  return out;
}
std::ostream& operator<<(std::ostream& out, StatusAction status_action) {
  out << ToString(status_action);
  return out;
}
std::ostream& operator<<(std::ostream& out, StatusReason status_reason) {
  out << ToString(status_reason);
  return out;
}
std::ostream& operator<<(std::ostream& out, TradingEvent trading_event) {
  out << ToString(trading_event);
  return out;
}
std::ostream& operator<<(std::ostream& out, TriState tri_state) {
  out << ToString(tri_state);
  return out;
}
std::ostream& operator<<(std::ostream& out,
                         VersionUpgradePolicy version_upgrade_policy) {
  out << ToString(version_upgrade_policy);
  return out;
}
std::ostream& operator<<(std::ostream& out, ErrorCode error_code) {
  out << ToString(error_code);
  return out;
}
std::ostream& operator<<(std::ostream& out, SystemCode system_code) {
  out << ToString(system_code);
  return out;
}

template <>
FeedMode FromString(const std::string& str) {
  if (str == "historical") {
    return FeedMode::Historical;
  }
  if (str == "historical-streaming") {
    return FeedMode::HistoricalStreaming;
  }
  if (str == "live") {
    return FeedMode::Live;
  }
  throw InvalidArgumentError{"FromString<FeedMode>", "str",
                             "unknown value '" + str + '\''};
}

template <>
SplitDuration FromString(const std::string& str) {
  if (str == "day") {
    return SplitDuration::Day;
  }
  if (str == "week") {
    return SplitDuration::Week;
  }
  if (str == "month") {
    return SplitDuration::Month;
  }
  if (str == "none") {
    return SplitDuration::None;
  }
  throw InvalidArgumentError{"FromString<SplitInterval>", "str",
                             "unknown value '" + str + '\''};
}

template <>
Delivery FromString(const std::string& str) {
  if (str == "download") {
    return Delivery::Download;
  }
  throw InvalidArgumentError{"FromString<Delivery>", "str",
                             "unknown value '" + str + '\''};
}

template <>
JobState FromString(const std::string& str) {
  if (str == "queued") {
    return JobState::Queued;
  }
  if (str == "processing") {
    return JobState::Processing;
  }
  if (str == "done") {
    return JobState::Done;
  }
  if (str == "expired") {
    return JobState::Expired;
  }
  throw InvalidArgumentError{"FromString<JobState>", "str",
                             "unknown value '" + str + '\''};
}

template <>
DatasetCondition FromString(const std::string& str) {
  if (str == "available") {
    return DatasetCondition::Available;
  }
  if (str == "degraded") {
    return DatasetCondition::Degraded;
  }
  if (str == "pending") {
    return DatasetCondition::Pending;
  }
  if (str == "missing") {
    return DatasetCondition::Missing;
  }
  throw InvalidArgumentError{"FromString<DatasetCondition>", "str",
                             "unknown value '" + str + '\''};
}

template <>
RType FromString(const std::string& str) {
  if (str == "mbp-0") {
    return RType::Mbp0;
  }
  if (str == "mbp-1") {
    return RType::Mbp1;
  }
  if (str == "mbp-10") {
    return RType::Mbp10;
  }
  if (str == "ohlcv-deprecated") {
    return RType::OhlcvDeprecated;
  }
  if (str == "ohlcv-1s") {
    return RType::Ohlcv1S;
  }
  if (str == "ohlcv-1m") {
    return RType::Ohlcv1M;
  }
  if (str == "ohlcv-1h") {
    return RType::Ohlcv1H;
  }
  if (str == "ohlcv-1d") {
    return RType::Ohlcv1D;
  }
  if (str == "ohlcv-eod") {
    return RType::OhlcvEod;
  }
  if (str == "status") {
    return RType::Status;
  }
  if (str == "instrument-def") {
    return RType::InstrumentDef;
  }
  if (str == "imbalance") {
    return RType::Imbalance;
  }
  if (str == "error") {
    return RType::Error;
  }
  if (str == "symbol-mapping") {
    return RType::SymbolMapping;
  }
  if (str == "system") {
    return RType::System;
  }
  if (str == "statistics") {
    return RType::Statistics;
  }
  if (str == "mbo") {
    return RType::Mbo;
  }
  if (str == "cmbp-1") {
    return RType::Cmbp1;
  }
  if (str == "cbbo-1s") {
    return RType::Cbbo1S;
  }
  if (str == "cbbo-1m") {
    return RType::Cbbo1M;
  }
  if (str == "tcbbo") {
    return RType::Tcbbo;
  }
  if (str == "bbo-1s") {
    return RType::Bbo1S;
  }
  if (str == "bbo-1m") {
    return RType::Bbo1M;
  }
  throw InvalidArgumentError{"FromString<RType>", "str",
                             "unknown value '" + str + '\''};
}
template <>
SType FromString(const std::string& str) {
  if (str == "instrument_id" || str == "product_id") {
    return SType::InstrumentId;
  }
  if (str == "raw_symbol" || str == "native") {
    return SType::RawSymbol;
  }
  if (str == "smart") {
    return SType::Smart;
  }
  if (str == "continuous") {
    return SType::Continuous;
  }
  if (str == "parent") {
    return SType::Parent;
  }
  if (str == "nasdaq_symbol" || str == "nasdaq") {
    return SType::NasdaqSymbol;
  }
  if (str == "cms_symbol" || str == "cms") {
    return SType::CmsSymbol;
  }
  if (str == "isin") {
    return SType::Isin;
  }
  if (str == "us_code") {
    return SType::UsCode;
  }
  if (str == "bbg_comp_id") {
    return SType::BbgCompId;
  }
  if (str == "bbg_comp_ticker") {
    return SType::BbgCompTicker;
  }
  if (str == "figi") {
    return SType::Figi;
  }
  if (str == "figi_ticker") {
    return SType::FigiTicker;
  }
  throw InvalidArgumentError{"FromString<SType>", "str",
                             "unknown value '" + str + '\''};
}
template <>
Schema FromString(const std::string& str) {
  if (str == "mbo") {
    return Schema::Mbo;
  }
  if (str == "mbp-1") {
    return Schema::Mbp1;
  }
  if (str == "mbp-10") {
    return Schema::Mbp10;
  }
  if (str == "tbbo") {
    return Schema::Tbbo;
  }
  if (str == "trades") {
    return Schema::Trades;
  }
  if (str == "ohlcv-1s") {
    return Schema::Ohlcv1S;
  }
  if (str == "ohlcv-1m") {
    return Schema::Ohlcv1M;
  }
  if (str == "ohlcv-1h") {
    return Schema::Ohlcv1H;
  }
  if (str == "ohlcv-1d") {
    return Schema::Ohlcv1D;
  }
  if (str == "definition") {
    return Schema::Definition;
  }
  if (str == "statistics") {
    return Schema::Statistics;
  }
  if (str == "status") {
    return Schema::Status;
  }
  if (str == "imbalance") {
    return Schema::Imbalance;
  }
  if (str == "ohlcv-eod") {
    return Schema::OhlcvEod;
  }
  if (str == "cmbp-1") {
    return Schema::Cmbp1;
  }
  if (str == "cbbo-1s") {
    return Schema::Cbbo1S;
  }
  if (str == "cbbo-1m") {
    return Schema::Cbbo1M;
  }
  if (str == "tcbbo") {
    return Schema::Tcbbo;
  }
  if (str == "bbo-1s") {
    return Schema::Bbo1S;
  }
  if (str == "bbo-1m") {
    return Schema::Bbo1M;
  }
  throw InvalidArgumentError{"FromString<Schema>", "str",
                             "unknown value '" + str + '\''};
}
template <>
Encoding FromString(const std::string& str) {
  if (str == "dbn" || str == "dbz") {
    return Encoding::Dbn;
  }
  if (str == "csv") {
    return Encoding::Csv;
  }
  if (str == "json") {
    return Encoding::Json;
  }
  throw InvalidArgumentError{"FromString<Encoding>", "str",
                             "unknown value '" + str + '\''};
}
template <>
Compression FromString(const std::string& str) {
  if (str == "none") {
    return Compression::None;
  }
  if (str == "zstd") {
    return Compression::Zstd;
  }
  throw InvalidArgumentError{"FromString<Compression>", "str",
                             "unknown value '" + str + '\''};
}
template <>
ErrorCode FromString(const std::string& str) {
  if (str == "auth_failed") {
    return ErrorCode::AuthFailed;
  }
  if (str == "api_key_deactivated") {
    return ErrorCode::ApiKeyDeactivated;
  }
  if (str == "connection_limit_exceeded") {
    return ErrorCode::ConnectionLimitExceeded;
  }
  if (str == "symbol_resolution_failed") {
    return ErrorCode::SymbolResolutionFailed;
  }
  if (str == "invalid_subscription") {
    return ErrorCode::InvalidSubscription;
  }
  if (str == "internal_error") {
    return ErrorCode::InternalError;
  }
  throw InvalidArgumentError{"FromString<ErrorCode>", "str",
                             "unknown value '" + str + '\''};
}
template <>
SystemCode FromString(const std::string& str) {
  if (str == "heartbeat") {
    return SystemCode::Heartbeat;
  }
  if (str == "subscription_ack") {
    return SystemCode::SubscriptionAck;
  }
  if (str == "slow_reader_warning") {
    return SystemCode::SlowReaderWarning;
  }
  if (str == "replay_completed") {
    return SystemCode::ReplayCompleted;
  }
  if (str == "end_of_interval") {
    return SystemCode::EndOfInterval;
  }
  throw InvalidArgumentError{"FromString<SystemCode>", "str",
                             "unknown value '" + str + '\''};
}
}  // namespace databento
