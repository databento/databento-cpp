#pragma once

#include <cstdint>
#include <iosfwd>
#include <string>

#include "databento/publishers.hpp"  // FromString

namespace databento {
// Represents a historical data center gateway location.
enum class HistoricalGateway : std::uint8_t {
  Bo1,
};

// Represents a data feed mode.
enum class FeedMode : std::uint8_t {
  Historical,
  HistoricalStreaming,
  Live,
};

// Represents a data record schema.
enum class Schema : std::uint16_t {
  Mbo = 0,
  Mbp1 = 1,
  Mbp10 = 2,
  Tbbo = 3,
  Trades = 4,
  Ohlcv1S = 5,
  Ohlcv1M = 6,
  Ohlcv1H = 7,
  Ohlcv1D = 8,
  Definition = 9,
  Statistics = 10,
  Status = 11,
  Imbalance = 12,
  Cmbp1 = 14,
  Cbbo1S = 15,
  Cbbo1M = 16,
  Tcbbo = 17,
  Bbo1S = 18,
  Bbo1M = 19,
};

// Represents a data output encoding.
enum class Encoding : std::uint8_t {
  Dbn = 0,
  Csv = 1,
  Json = 2,
};

// Represents a data compression format (if any).
enum class Compression : std::uint8_t {
  None = 0,
  Zstd = 1,
};

// Represents a symbology type.
namespace stype {
enum SType : std::uint8_t {
  // Symbology using a unique numeric ID.
  InstrumentId = 0,
  // Symbology using the original symbols provided by the publisher.
  RawSymbol = 1,
  // A Databento-specific symbology where one symbol may point to different
  // instruments at different points of time, e.g. to always refer to the front
  // month future.
  Continuous = 3,
  // A Databento-specific symbology for referring to a group of symbols by one
  // "parent" symbol, e.g. ES.FUT to refer to all ES futures.
  Parent = 4,
  // Symbology for US equities using NASDAQ Integrated suffix conventions.
  NasdaqSymbol = 5,
  // Symbology for US equities using CMS suffix conventions.
  CmsSymbol = 6,
};
}  // namespace stype
using SType = stype::SType;

// Represents the duration of time at which batch files will be split.
enum class SplitDuration : std::uint8_t {
  Day = 0,
  Week,
  Month,
  None,
};

// Represents how a batch job will be delivered.
enum class Delivery : std::uint8_t {
  Download,
  S3,
  Disk,
};

// The current state of a batch job.
enum class JobState : std::uint8_t {
  Received,
  Queued,
  Processing,
  Done,
  Expired,
};

// The condition of a dataset at a point in time.
enum class DatasetCondition : std::uint8_t {
  Available,
  Degraded,
  Pending,
  Missing,
  Intraday,
};

// Sentinel values for different DBN record types.
//
// Additional rtypes may be added in the future.
namespace rtype {
enum RType : std::uint8_t {
  Mbp0 = 0x00,
  Mbp1 = 0x01,
  Mbp10 = 0x0A,
  // Deprecated in 0.4.0. Separated into separate rtypes for each OHLCV schema.
  OhlcvDeprecated = 0x11,
  Ohlcv1S = 0x20,
  Ohlcv1M = 0x21,
  Ohlcv1H = 0x22,
  Ohlcv1D = 0x23,
  Status = 0x12,
  InstrumentDef = 0x13,
  Imbalance = 0x14,
  Error = 0x15,
  SymbolMapping = 0x16,
  System = 0x17,
  Statistics = 0x18,
  Mbo = 0xA0,
  Cmbp1 = 0xB1,
  Cbbo1S = 0xC0,
  Cbbo1M = 0xC1,
  Tcbbo = 0xC2,
  Bbo1S = 0xC3,
  Bbo1M = 0xC4,
};
}  // namespace rtype
using rtype::RType;

// An order event or order book operation.
//
// For example usage see:
// - https://databento.com/docs/examples/order-book/order-actions
// - https://databento.com/docs/examples/order-book/order-tracking
namespace action {
// enum because future variants may be added in the future.
enum Action : char {
  // An existing order was modified.
  Modify = 'M',
  // A trade executed.
  Trade = 'T',
  // An existing order was filled.
  Fill = 'F',
  // An order was cancelled.
  Cancel = 'C',
  // A new order was added.
  Add = 'A',
  // Reset the book; clear all orders for an instrument.
  Clear = 'R',
  // Has no effect on the book, but may carry `flags` or other information.
  None = 'N',
};
}  // namespace action
using action::Action;

// A side of the market. The side of the market for resting orders, or the side
// of the aggressor for trades.
enum class Side : char {
  // A sell order.
  Ask = 'A',
  // A buy order.
  Bid = 'B',
  // None or unknown.
  None = 'N',
};

namespace instrument_class {
// enum because future variants may be added in the future.
enum InstrumentClass : char {
  Bond = 'B',
  Call = 'C',
  Future = 'F',
  Stock = 'K',
  MixedSpread = 'M',
  Put = 'P',
  FutureSpread = 'S',
  OptionSpread = 'T',
  FxSpot = 'X',
  CommoditySpot = 'Y',
};
}  // namespace instrument_class
using instrument_class::InstrumentClass;

namespace match_algorithm {
enum MatchAlgorithm : char {
  Undefined = ' ',
  Fifo = 'F',
  Configurable = 'K',
  ProRata = 'C',
  FifoLmm = 'T',
  ThresholdProRata = 'O',
  FifoTopLmm = 'S',
  ThresholdProRataLmm = 'Q',
  EurodollarFutures = 'Y',
  TimeProRata = 'P',
};
}  // namespace match_algorithm
using match_algorithm::MatchAlgorithm;

namespace security_update_action {
enum SecurityUpdateAction : char {
  Add = 'A',
  Modify = 'M',
  Delete = 'D',
};
}
using security_update_action::SecurityUpdateAction;

namespace user_defined_instrument {
enum UserDefinedInstrument : char {
  No = 'N',
  Yes = 'Y',
};
}
using user_defined_instrument::UserDefinedInstrument;

namespace stat_type {
// The type of statistic contained in a StatMsg.
enum StatType : std::uint16_t {
  // The price of the first trade of an instrument. `price` will be set.
  // `quantity` will be set when provided by the venue.
  OpeningPrice = 1,
  // The probable price of the first trade of an instrument published during
  // pre-open. Both `price` and `quantity` will be set.
  IndicativeOpeningPrice = 2,
  // The settlement price of an instrument. `price` will be set and `flags`
  // indicate whether the price is final or preliminary and actual or
  // theoretical. `ts_ref` will indicate the trading date of the settlement
  // price.
  SettlementPrice = 3,
  // The lowest trade price of an instrument during the trading session.
  // `price` will be set.
  TradingSessionLowPrice = 4,
  // The highest trade price of an instrument during the trading session.
  // `price` will be set.
  TradingSessionHighPrice = 5,
  // The number of contracts cleared for an instrument on the previous trading
  // date. `quantity` will be set. `ts_ref` will indicate the trading date of
  // the volume.
  ClearedVolume = 6,
  // The lowest offer price for an instrument during the trading session.
  // `price` will be set.
  LowestOffer = 7,
  // The highest bid price for an instrument during the trading session.
  // `price` will be set.
  HighestBid = 8,
  // The current number of outstanding contracts of an instrument. `quantity`
  // will be set. `ts_ref` will indicate the trading date for which the open
  // interest was calculated.
  OpenInterest = 9,
  // The volume-weighted average price (VWAP) for a fixing period. `price` will
  // be set.
  FixingPrice = 10,
  // The last trade price during a trading session. `price` will be set.
  // `quantity` will be set when provided by the venue.
  ClosePrice = 11,
  // The change in price from the close price of the previous trading session to
  // the most recent trading session. `price` will be set.
  NetChange = 12,
  /// The volume-weighted average price (VWAP) during the trading session.
  /// `price` will be set to the VWAP while `quantity` will be the traded
  /// volume.
  Vwap = 13,
  // The implied volatility associated with the settlement price. `price` will
  // be set with the standard precision.
  Volatility = 14,
  // The option delta associated with the settlement price. `price` will be set
  // with the standard precision.
  Delta = 15,
  // The auction uncrossing price. This is used for auctions that are neither
  // the
  // official opening auction nor the official closing auction. `price` will be
  // set. `quantity` will be set when provided by the venue.
  UncrossingPrice = 16,

};
}  // namespace stat_type
using stat_type::StatType;

// The type of StatMsg update.
enum class StatUpdateAction : std::uint8_t {
  New = 1,
  Delete = 2,
};

// How to handle decoding DBN data from a prior version.
enum class VersionUpgradePolicy : std::uint8_t {
  AsIs = 0,
  UpgradeToV2 = 1,
};

namespace status_action {
enum StatusAction : std::uint16_t {
  // No change.
  None = 0,
  // The instrument is in a pre-open period.
  PreOpen = 1,
  // The instrument is in a pre-cross period.
  PreCross = 2,
  // The instrument is quoting but not trading.
  Quoting = 3,
  // The instrument is in a cross/auction.
  Cross = 4,
  // The instrument is being opened through a trading rotation.
  Rotation = 5,
  // A new price indication is available for the instrument.
  NewPriceIndication = 6,
  // The instrument is trading.
  Trading = 7,
  // Trading in the instrument has been halted.
  Halt = 8,
  // Trading in the instrument has been paused.
  Pause = 9,
  // Trading in the instrument has been suspended.
  Suspend = 10,
  // The instrument is in a pre-close period.
  PreClose = 11,
  // Trading in the instrument has closed.
  Close = 12,
  // The instrument is in a post-close period.
  PostClose = 13,
  // A change in short-selling restrictions.
  SsrChange = 14,
  // The instrument is not available for trading, either trading has closed or
  // been halted.
  NotAvailableForTrading = 15,
};
}  // namespace status_action
using status_action::StatusAction;

namespace status_reason {
enum StatusReason : std::uint16_t {
  // No reason is given.
  None = 0,
  // The change in status occurred as scheduled.
  Scheduled = 1,
  // The instrument stopped due to a market surveillance intervention.
  SurveillanceIntervention = 2,
  // The status changed due to activity in the market.
  MarketEvent = 3,
  // The derivative instrument began trading.
  InstrumentActivation = 4,
  // The derivative instrument expired.
  InstrumentExpiration = 5,
  // Recovery in progress.
  RecoveryInProcess = 6,
  // The status change was caused by a regulatory action.
  Regulatory = 10,
  // The status change was caused by an administrative action.
  Administrative = 11,
  // The status change was caused by the issuer not being compliance with
  // regulatory requirements.
  NonCompliance = 12,
  // Trading halted because the issuer's filings are not current.
  FilingsNotCurrent = 13,
  // Trading halted due to an SEC trading suspension.
  SecTradingSuspension = 14,
  // The status changed because a new issue is available.
  NewIssue = 15,
  // The status changed because an issue is available.
  IssueAvailable = 16,
  // The status changed because the issue was reviewed.
  IssuesReviewed = 17,
  // The status changed because the filing requirements were satisfied.
  FilingReqsSatisfied = 18,
  // Relevant news is pending.
  NewsPending = 30,
  // Relevant news was released.
  NewsReleased = 31,
  // The news has been fully disseminated and times are available for the
  // resumption in quoting and trading.
  NewsAndResumptionTimes = 32,
  // The relevants news was not forthcoming.
  NewsNotForthcoming = 33,
  // Halted for order imbalance.
  OrderImbalance = 40,
  // The instrument hit limit up or limit down.
  LuldPause = 50,
  // An operational issue occurred with the venue.
  Operational = 60,
  // The status changed until the exchange receives additional information.
  AdditionalInformationRequested = 70,
  // Trading halted due to merger becoming effective.
  MergerEffective = 80,
  // Trading is halted in an ETF due to conditions with the component
  // securities.
  Etf = 90,
  // Trading is halted for a corporate action.
  CorporateAction = 100,
  // Trading is halted because the instrument is a new offering.
  NewSecurityOffering = 110,
  // Halted due to the market-wide circuit breaker level 1.
  MarketWideHaltLevel1 = 120,
  // Halted due to the market-wide circuit breaker level 2.
  MarketWideHaltLevel2 = 121,
  // Halted due to the market-wide circuit breaker level 3.
  MarketWideHaltLevel3 = 122,
  // Halted due to the carryover of a market-wide circuit breaker from the
  // previous trading day.
  MarketWideHaltCarryover = 123,
  // Resumption due to the end of the a market-wide circuit breaker halt.
  MarketWideHaltResumption = 124,
  // Halted because quotation is not available.
  QuotationNotAvailable = 130,
};
}  // namespace status_reason
using status_reason::StatusReason;

namespace trading_event {
enum TradingEvent : std::uint16_t {
  // No additional information given.
  None = 0,
  // Order entry and modification are not allowed.
  NoCancel = 1,
  // A change of trading session occurred. Daily statistics are reset.
  ChangeTradingSession = 2,
  // Implied matching is available.
  ImpliedMatchingOn = 3,
  // Implied matching is not available.
  ImpliedMatchingOff = 4,
};
}  // namespace trading_event
using trading_event::TradingEvent;

// An enum for representing unknown, true, or false values. Equivalent to a
// std::optional<bool>.
enum class TriState : char {
  // The value is not applicable or not known.
  NotAvailable = '~',
  // False
  No = 'N',
  // True
  Yes = 'Y',
};

// Convert a HistoricalGateway to a URL.
const char* UrlFromGateway(HistoricalGateway gateway);

const char* ToString(Schema schema);
const char* ToString(Encoding encoding);
const char* ToString(FeedMode mode);
const char* ToString(Compression compression);
const char* ToString(SType stype);
const char* ToString(SplitDuration duration_interval);
const char* ToString(Delivery delivery);
const char* ToString(JobState state);
const char* ToString(DatasetCondition condition);
const char* ToString(RType rtype);
const char* ToString(Action action);
const char* ToString(Side side);
const char* ToString(InstrumentClass instrument_class);
const char* ToString(MatchAlgorithm match_algorithm);
const char* ToString(SecurityUpdateAction update_action);
const char* ToString(UserDefinedInstrument user_def_instr);
const char* ToString(StatType stat_type);
const char* ToString(StatUpdateAction stat_update_action);
const char* ToString(StatusAction status_action);
const char* ToString(StatusReason status_reason);
const char* ToString(TradingEvent trading_event);
const char* ToString(TriState tri_state);
const char* ToString(VersionUpgradePolicy upgrade_policy);

std::ostream& operator<<(std::ostream& out, Schema schema);
std::ostream& operator<<(std::ostream& out, Encoding encoding);
std::ostream& operator<<(std::ostream& out, FeedMode mode);
std::ostream& operator<<(std::ostream& out, Compression compression);
std::ostream& operator<<(std::ostream& out, SType stype);
std::ostream& operator<<(std::ostream& out, SplitDuration duration_interval);
std::ostream& operator<<(std::ostream& out, Delivery delivery);
std::ostream& operator<<(std::ostream& out, JobState state);
std::ostream& operator<<(std::ostream& out, DatasetCondition condition);
std::ostream& operator<<(std::ostream& out, RType rtype);
std::ostream& operator<<(std::ostream& out, Action action);
std::ostream& operator<<(std::ostream& out, Side side);
std::ostream& operator<<(std::ostream& out, InstrumentClass instrument_class);
std::ostream& operator<<(std::ostream& out, MatchAlgorithm match_algorithm);
std::ostream& operator<<(std::ostream& out, SecurityUpdateAction update_action);
std::ostream& operator<<(std::ostream& out,
                         UserDefinedInstrument user_def_instr);
std::ostream& operator<<(std::ostream& out, StatType stat_type);
std::ostream& operator<<(std::ostream& out,
                         StatUpdateAction stat_update_action);
std::ostream& operator<<(std::ostream& out, StatusAction status_action);
std::ostream& operator<<(std::ostream& out, StatusReason status_reason);
std::ostream& operator<<(std::ostream& out, TradingEvent trading_event);
std::ostream& operator<<(std::ostream& out, TriState tri_state);
std::ostream& operator<<(std::ostream& out,
                         VersionUpgradePolicy upgrade_policy);

template <>
Schema FromString(const std::string& str);
template <>
Encoding FromString(const std::string& str);
template <>
FeedMode FromString(const std::string& str);
template <>
Compression FromString(const std::string& str);
template <>
SType FromString(const std::string& str);
template <>
SplitDuration FromString(const std::string& str);
template <>
Delivery FromString(const std::string& str);
template <>
JobState FromString(const std::string& str);
template <>
DatasetCondition FromString(const std::string& str);
}  // namespace databento
