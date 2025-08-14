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
};

// The current state of a batch job.
enum class JobState : std::uint8_t {
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
};

// A record type sentinel.
namespace r_type {
enum RType : std::uint8_t {
  // none
  Mbp0 = 0x00,
  // none
  Mbp1 = 0x01,
  // Denotes a market-by-price record with a book depth of 10.
  Mbp10 = 0x0A,
  // Denotes an open, high, low, close, and volume record at an unspecified cadence.
  OhlcvDeprecated = 0x11,
  // Denotes an open, high, low, close, and volume record at a 1-second cadence.
  Ohlcv1S = 0x20,
  // Denotes an open, high, low, close, and volume record at a 1-minute cadence.
  Ohlcv1M = 0x21,
  // Denotes an open, high, low, close, and volume record at an hourly cadence.
  Ohlcv1H = 0x22,
  // Denotes an open, high, low, close, and volume record at a daily cadence
  // based on the UTC date.
  Ohlcv1D = 0x23,
  // Denotes an open, high, low, close, and volume record at a daily cadence
  // based on the end of the trading session.
  OhlcvEod = 0x24,
  // Denotes an exchange status record.
  Status = 0x12,
  // Denotes an instrument definition record.
  InstrumentDef = 0x13,
  // Denotes an order imbalance record.
  Imbalance = 0x14,
  // Denotes an error from gateway.
  Error = 0x15,
  // Denotes a symbol mapping record.
  SymbolMapping = 0x16,
  // Denotes a non-error message from the gateway. Also used for heartbeats.
  System = 0x17,
  // Denotes a statistics record from the publisher (not calculated by Databento).
  Statistics = 0x18,
  // Denotes a market-by-order record.
  Mbo = 0xA0,
  // Denotes a consolidated best bid and offer record.
  Cmbp1 = 0xB1,
  // Denotes a consolidated best bid and offer record subsampled on a one-second
  // interval.
  Cbbo1S = 0xC0,
  // Denotes a consolidated best bid and offer record subsampled on a one-minute
  // interval.
  Cbbo1M = 0xC1,
  // Denotes a consolidated best bid and offer trade record containing the
  // consolidated BBO before the trade
  Tcbbo = 0xC2,
  // Denotes a best bid and offer record subsampled on a one-second interval.
  Bbo1S = 0xC3,
  // Denotes a best bid and offer record subsampled on a one-minute interval.
  Bbo1M = 0xC4,
};
}  // namespace r_type
using r_type::RType;

// A
// [side](https://databento.com/docs/standards-and-conventions/common-fields-enums-types)
// of the market. The side of the market for resting orders, or the side of the
// aggressor for trades.
namespace side {
enum Side : char {
  // A sell order or sell aggressor in a trade.
  Ask = 'A',
  // A buy order or a buy aggressor in a trade.
  Bid = 'B',
  // No side specified by the original source.
  None = 'N',
};
}  // namespace side
using side::Side;

// An [order event or order book
// operation](https://databento.com/docs/api-reference-historical/basics/schemas-and-conventions).
//
// For example usage see:
// - [Order actions](https://databento.com/docs/examples/order-book/order-actions)
// - [Order tracking](https://databento.com/docs/examples/order-book/order-tracking)
namespace action {
enum Action : char {
  // An existing order was modified: price and/or size.
  Modify = 'M',
  // An aggressing order traded. Does not affect the book.
  Trade = 'T',
  // An existing order was filled. Does not affect the book.
  Fill = 'F',
  // An order was fully or partially cancelled.
  Cancel = 'C',
  // A new order was added to the book.
  Add = 'A',
  // Reset the book; clear all orders for an instrument.
  Clear = 'R',
  // Has no effect on the book, but may carry `flags` or other information.
  None = 'N',
};
}  // namespace action
using action::Action;

// The class of instrument.
//
// For example usage see
// [Getting options with their
// underlying](https://databento.com/docs/examples/options/options-and-futures).
namespace instrument_class {
enum InstrumentClass : char {
  // A bond.
  Bond = 'B',
  // A call option.
  Call = 'C',
  // A future.
  Future = 'F',
  // A stock.
  Stock = 'K',
  // A spread composed of multiple instrument classes.
  MixedSpread = 'M',
  // A put option.
  Put = 'P',
  // A spread composed of futures.
  FutureSpread = 'S',
  // A spread composed of options.
  OptionSpread = 'T',
  // A foreign exchange spot.
  FxSpot = 'X',
  // A commodity being traded for immediate delivery.
  CommoditySpot = 'Y',
};
}  // namespace instrument_class
using instrument_class::InstrumentClass;

// The type of matching algorithm used for the instrument at the exchange.
namespace match_algorithm {
enum MatchAlgorithm : char {
  // No matching algorithm was specified.
  Undefined = ' ',
  // First-in-first-out matching.
  Fifo = 'F',
  // A configurable match algorithm.
  Configurable = 'K',
  // Trade quantity is allocated to resting orders based on a pro-rata percentage:
  // resting order quantity divided by total quantity.
  ProRata = 'C',
  // Like `FIFO` but with LMM allocations prior to FIFO allocations.
  FifoLmm = 'T',
  // Like `PRO_RATA` but includes a configurable allocation to the first order that
  // improves the market.
  ThresholdProRata = 'O',
  // Like `FIFO_LMM` but includes a configurable allocation to the first order that
  // improves the market.
  FifoTopLmm = 'S',
  // Like `THRESHOLD_PRO_RATA` but includes a special priority to LMMs.
  ThresholdProRataLmm = 'Q',
  // Special variant used only for Eurodollar futures on CME.
  EurodollarFutures = 'Y',
  // Trade quantity is shared between all orders at the best price. Orders with the
  // highest time priority receive a higher matched quantity.
  TimeProRata = 'P',
  // A two-pass FIFO algorithm. The first pass fills the Institutional Group the
  // aggressing
  // order is associated with. The second pass matches orders without an Institutional
  // Group
  // association. See [CME
  // documentation](https://cmegroupclientsite.atlassian.net/wiki/spaces/EPICSANDBOX/pages/457217267#InstitutionalPrioritizationMatchAlgorithm).
  InstitutionalPrioritization = 'V',
};
}  // namespace match_algorithm
using match_algorithm::MatchAlgorithm;

// Whether the instrument is user-defined.
//
// For example usage see
// [Getting options with their
// underlying](https://databento.com/docs/examples/options/options-and-futures).
namespace user_defined_instrument {
enum UserDefinedInstrument : char {
  // The instrument is not user-defined.
  No = 'N',
  // The instrument is user-defined.
  Yes = 'Y',
};
}  // namespace user_defined_instrument
using user_defined_instrument::UserDefinedInstrument;

// The type of `InstrumentDefMsg` update.
namespace security_update_action {
enum SecurityUpdateAction : char {
  // A new instrument definition.
  Add = 'A',
  // A modified instrument definition of an existing one.
  Modify = 'M',
  // Removal of an instrument definition.
  Delete = 'D',
};
}  // namespace security_update_action
using security_update_action::SecurityUpdateAction;

// A symbology type. Refer to the
// [symbology
// documentation](https://databento.com/docs/api-reference-historical/basics/symbology)
// for more information.
namespace s_type {
enum SType : std::uint8_t {
  // Symbology using a unique numeric ID.
  InstrumentId = 0,
  // Symbology using the original symbols provided by the publisher.
  RawSymbol = 1,
  // A set of Databento-specific symbologies for referring to groups of symbols.
  Smart = 2,
  // A Databento-specific symbology where one symbol may point to different
  // instruments at different points of time, e.g. to always refer to the front month
  // future.
  Continuous = 3,
  // A Databento-specific symbology for referring to a group of symbols by one
  // "parent" symbol, e.g. ES.FUT to refer to all ES futures.
  Parent = 4,
  // Symbology for US equities using NASDAQ Integrated suffix conventions.
  NasdaqSymbol = 5,
  // Symbology for US equities using CMS suffix conventions.
  CmsSymbol = 6,
  // Symbology using International Security Identification Numbers (ISIN) - ISO 6166.
  Isin = 7,
  // Symbology using US domestic Committee on Uniform Securities Identification
  // Procedure (CUSIP) codes.
  UsCode = 8,
  // Symbology using Bloomberg composite global IDs.
  BbgCompId = 9,
  // Symbology using Bloomberg composite tickers.
  BbgCompTicker = 10,
  // Symbology using Bloomberg FIGI exchange level IDs.
  Figi = 11,
  // Symbology using Bloomberg exchange level tickers.
  FigiTicker = 12,
};
}  // namespace s_type
using s_type::SType;

// A data record schema.
//
// Each schema has a particular record type associated with it.
//
// See [List of supported market data
// schemas](https://databento.com/docs/schemas-and-data-formats/whats-a-schema) for an
// overview of the differences and use cases of each schema.
enum class Schema : std::uint16_t {
  // Market by order.
  Mbo = 0,
  // Market by price with a book depth of 1.
  Mbp1 = 1,
  // Market by price with a book depth of 10.
  Mbp10 = 2,
  // All trade events with the best bid and offer (BBO) immediately **before** the
  // effect of
  // the trade.
  Tbbo = 3,
  // All trade events.
  Trades = 4,
  // Open, high, low, close, and volume at a one-second interval.
  Ohlcv1S = 5,
  // Open, high, low, close, and volume at a one-minute interval.
  Ohlcv1M = 6,
  // Open, high, low, close, and volume at an hourly interval.
  Ohlcv1H = 7,
  // Open, high, low, close, and volume at a daily interval based on the UTC date.
  Ohlcv1D = 8,
  // Instrument definitions.
  Definition = 9,
  // Additional data disseminated by publishers.
  Statistics = 10,
  // Trading status events.
  Status = 11,
  // Auction imbalance events.
  Imbalance = 12,
  // Open, high, low, close, and volume at a daily cadence based on the end of the
  // trading
  // session.
  OhlcvEod = 13,
  // Consolidated best bid and offer.
  Cmbp1 = 14,
  // Consolidated best bid and offer subsampled at one-second intervals, in addition to
  // trades.
  Cbbo1S = 15,
  // Consolidated best bid and offer subsampled at one-minute intervals, in addition to
  // trades.
  Cbbo1M = 16,
  // All trade events with the consolidated best bid and offer (CBBO) immediately
  // **before**
  // the effect of the trade.
  Tcbbo = 17,
  // Best bid and offer subsampled at one-second intervals, in addition to trades.
  Bbo1S = 18,
  // Best bid and offer subsampled at one-minute intervals, in addition to trades.
  Bbo1M = 19,
};

// A data encoding format.
enum class Encoding : std::uint8_t {
  // Databento Binary Encoding.
  Dbn = 0,
  // Comma-separated values.
  Csv = 1,
  // JavaScript object notation.
  Json = 2,
};

// A compression format or none if uncompressed.
enum class Compression : std::uint8_t {
  // Uncompressed.
  None = 0,
  // Zstandard compressed.
  Zstd = 1,
};

// The type of statistic contained in a `StatMsg`.
namespace stat_type {
enum StatType : std::uint16_t {
  // The price of the first trade of an instrument. `price` will be set.
  // `quantity` will be set when provided by the venue.
  OpeningPrice = 1,
  // The probable price of the first trade of an instrument published during pre-
  // open. Both `price` and `quantity` will be set.
  IndicativeOpeningPrice = 2,
  // The settlement price of an instrument. `price` will be set and `flags` indicate
  // whether the price is final or preliminary and actual or theoretical. `ts_ref`
  // will indicate the trading date of the settlement price.
  SettlementPrice = 3,
  // The lowest trade price of an instrument during the trading session. `price` will
  // be set.
  TradingSessionLowPrice = 4,
  // The highest trade price of an instrument during the trading session. `price` will
  // be set.
  TradingSessionHighPrice = 5,
  // The number of contracts cleared for an instrument on the previous trading date.
  // `quantity` will be set. `ts_ref` will indicate the trading date of the volume.
  ClearedVolume = 6,
  // The lowest offer price for an instrument during the trading session. `price`
  // will be set.
  LowestOffer = 7,
  // The highest bid price for an instrument during the trading session. `price`
  // will be set.
  HighestBid = 8,
  // The current number of outstanding contracts of an instrument. `quantity` will
  // be set. `ts_ref` will indicate the trading date for which the open interest was
  // calculated.
  OpenInterest = 9,
  // The volume-weighted average price (VWAP) for a fixing period. `price` will be
  // set.
  FixingPrice = 10,
  // The last trade price during a trading session. `price` will be set.
  // `quantity` will be set when provided by the venue.
  ClosePrice = 11,
  // The change in price from the close price of the previous trading session to the
  // most recent trading session. `price` will be set.
  NetChange = 12,
  // The volume-weighted average price (VWAP) during the trading session.
  // `price` will be set to the VWAP while `quantity` will be the traded
  // volume.
  Vwap = 13,
  // The implied volatility associated with the settlement price. `price` will be set
  // with the standard precision.
  Volatility = 14,
  // The option delta associated with the settlement price. `price` will be set with
  // the standard precision.
  Delta = 15,
  // The auction uncrossing price. This is used for auctions that are neither the
  // official opening auction nor the official closing auction. `price` will be set.
  // `quantity` will be set when provided by the venue.
  UncrossingPrice = 16,
};
}  // namespace stat_type
using stat_type::StatType;

// The type of `StatMsg` update.
namespace stat_update_action {
enum StatUpdateAction : std::uint8_t {
  // A new statistic.
  New = 1,
  // A removal of a statistic.
  Delete = 2,
};
}  // namespace stat_update_action
using stat_update_action::StatUpdateAction;

// The primary enum for the type of `StatusMsg` update.
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
  // The instrument is not available for trading, either trading has closed or been
  // halted.
  NotAvailableForTrading = 15,
};
}  // namespace status_action
using status_action::StatusAction;

// The secondary enum for a `StatusMsg` update, explains
// the cause of a halt or other change in `action`.
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
  // The status change was caused by the issuer not being compliance with regulatory
  // requirements.
  NonCompliance = 12,
  // Trading halted because the issuer's filings are not current.
  FilingsNotCurrent = 13,
  // Trading halted due to an SEC trading suspension.
  SecTradingSuspension = 14,
  // The status changed because a new issue is available.
  NewIssue = 15,
  // The status changed because an issue is available.
  IssueAvailable = 16,
  // The status changed because the issue(s) were reviewed.
  IssuesReviewed = 17,
  // The status changed because the filing requirements were satisfied.
  FilingReqsSatisfied = 18,
  // Relevant news is pending.
  NewsPending = 30,
  // Relevant news was released.
  NewsReleased = 31,
  // The news has been fully disseminated and times are available for the resumption
  // in quoting and trading.
  NewsAndResumptionTimes = 32,
  // The relevant news was not forthcoming.
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
  // Trading is halted in an ETF due to conditions with the component securities.
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
  // Halted due to the carryover of a market-wide circuit breaker from the previous
  // trading day.
  MarketWideHaltCarryover = 123,
  // Resumption due to the end of a market-wide circuit breaker halt.
  MarketWideHaltResumption = 124,
  // Halted because quotation is not available.
  QuotationNotAvailable = 130,
};
}  // namespace status_reason
using status_reason::StatusReason;

// Further information about a status update.
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

// An enum for representing unknown, true, or false values.
namespace tri_state {
enum TriState : char {
  // The value is not applicable or not known.
  NotAvailable = '~',
  // False.
  No = 'N',
  // True.
  Yes = 'Y',
};
}  // namespace tri_state
using tri_state::TriState;

// How to handle decoding DBN data from other versions.
enum class VersionUpgradePolicy {
  // Decode data from all supported versions (less than or equal to
  // `DBN_VERSION`) as-is.
  AsIs,
  // Decode and convert data from DBN versions prior to version 2 to that version.
  // Attempting to decode data from newer versions will fail.
  UpgradeToV2,
  // Decode and convert data from DBN versions prior to version 3 to that version.
  // Attempting to decode data from newer versions (when they're introduced) will
  // fail.
  UpgradeToV3,
};

// An error code from the live subscription gateway.
namespace error_code {
enum ErrorCode : std::uint8_t {
  // The authentication step failed.
  AuthFailed = 1,
  // The user account or API key were deactivated.
  ApiKeyDeactivated = 2,
  // The user has exceeded their open connection limit.
  ConnectionLimitExceeded = 3,
  // One or more symbols failed to resolve.
  SymbolResolutionFailed = 4,
  // There was an issue with a subscription request (other than symbol resolution).
  InvalidSubscription = 5,
  // An error occurred in the gateway.
  InternalError = 6,
  Unset = 255,
};
}  // namespace error_code
using error_code::ErrorCode;

// A `SystemMsg` code indicating the type of message from the live
// subscription gateway.
namespace system_code {
enum SystemCode : std::uint8_t {
  // A message sent in the absence of other records to indicate the connection
  // remains open.
  Heartbeat = 0,
  // An acknowledgement of a subscription request.
  SubscriptionAck = 1,
  // The gateway has detected this session is falling behind real-time.
  SlowReaderWarning = 2,
  // Indicates a replay subscription has caught up with real-time data.
  ReplayCompleted = 3,
  // Signals that all records for interval-based schemas have been published for the
  // given timestamp.
  EndOfInterval = 4,
  Unset = 255,
};
}  // namespace system_code
using system_code::SystemCode;

// Convert a HistoricalGateway to a URL.
const char* UrlFromGateway(HistoricalGateway gateway);

const char* ToString(FeedMode mode);
const char* ToString(SplitDuration duration_interval);
const char* ToString(Delivery delivery);
const char* ToString(JobState state);
const char* ToString(DatasetCondition condition);
const char* ToString(RType r_type);
const char* ToString(Side side);
const char* ToString(Action action);
const char* ToString(InstrumentClass instrument_class);
const char* ToString(MatchAlgorithm match_algorithm);
const char* ToString(UserDefinedInstrument user_defined_instrument);
const char* ToString(SecurityUpdateAction security_update_action);
const char* ToString(SType s_type);
const char* ToString(Schema schema);
const char* ToString(Encoding encoding);
const char* ToString(Compression compression);
const char* ToString(StatType stat_type);
const char* ToString(StatUpdateAction stat_update_action);
const char* ToString(StatusAction status_action);
const char* ToString(StatusReason status_reason);
const char* ToString(TradingEvent trading_event);
const char* ToString(TriState tri_state);
const char* ToString(VersionUpgradePolicy version_upgrade_policy);
const char* ToString(ErrorCode error_code);
const char* ToString(SystemCode system_code);

std::ostream& operator<<(std::ostream& out, FeedMode mode);
std::ostream& operator<<(std::ostream& out, SplitDuration duration_interval);
std::ostream& operator<<(std::ostream& out, Delivery delivery);
std::ostream& operator<<(std::ostream& out, JobState state);
std::ostream& operator<<(std::ostream& out, DatasetCondition condition);
std::ostream& operator<<(std::ostream& out, RType r_type);
std::ostream& operator<<(std::ostream& out, Side side);
std::ostream& operator<<(std::ostream& out, Action action);
std::ostream& operator<<(std::ostream& out, InstrumentClass instrument_class);
std::ostream& operator<<(std::ostream& out, MatchAlgorithm match_algorithm);
std::ostream& operator<<(std::ostream& out,
                         UserDefinedInstrument user_defined_instrument);
std::ostream& operator<<(std::ostream& out,
                         SecurityUpdateAction security_update_action);
std::ostream& operator<<(std::ostream& out, SType s_type);
std::ostream& operator<<(std::ostream& out, Schema schema);
std::ostream& operator<<(std::ostream& out, Encoding encoding);
std::ostream& operator<<(std::ostream& out, Compression compression);
std::ostream& operator<<(std::ostream& out, StatType stat_type);
std::ostream& operator<<(std::ostream& out, StatUpdateAction stat_update_action);
std::ostream& operator<<(std::ostream& out, StatusAction status_action);
std::ostream& operator<<(std::ostream& out, StatusReason status_reason);
std::ostream& operator<<(std::ostream& out, TradingEvent trading_event);
std::ostream& operator<<(std::ostream& out, TriState tri_state);
std::ostream& operator<<(std::ostream& out,
                         VersionUpgradePolicy version_upgrade_policy);
std::ostream& operator<<(std::ostream& out, ErrorCode error_code);
std::ostream& operator<<(std::ostream& out, SystemCode system_code);

template <>
FeedMode FromString(const std::string& str);
template <>
SplitDuration FromString(const std::string& str);
template <>
Delivery FromString(const std::string& str);
template <>
JobState FromString(const std::string& str);
template <>
DatasetCondition FromString(const std::string& str);
template <>
RType FromString(const std::string& str);
template <>
SType FromString(const std::string& str);
template <>
Schema FromString(const std::string& str);
template <>
Encoding FromString(const std::string& str);
template <>
Compression FromString(const std::string& str);
template <>
ErrorCode FromString(const std::string& str);
template <>
SystemCode FromString(const std::string& str);
}  // namespace databento
