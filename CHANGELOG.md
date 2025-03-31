# Changelog

## 0.32.0 - 2025-04-01

### Enhancements
- Upgraded default date version to 3.0.3

### Breaking changes
- Upgraded default cpp-httplib version to 0.20.0 which requires OpenSSL >= 3.0

## 0.31.0 - 2025-03-18

### Enhancements
- Added new venues, datasets, and publishers for ICE Futures US, ICE Futures Europe
  (Financial products), Eurex, and European Energy Exchange (EEX)

## 0.30.0 - 2025-02-11

### Enhancements
- Added `Resubscribe()` methods to `LiveBlocking` and `LiveThreaded` to make it easier
  to resume a live session after losing the connection to the live gateway
- Added `Subscriptions()` getter methods to `LiveBlocking` and `LiveThreaded` for
  getting all active subscriptions
- Added `CommoditySpot` `InstrumentClass` variant

## 0.29.0 - 2025-02-04

### Enhancements
- Fixed documentation for using external versions of libraries (credit: @ElBellaCiao)

### Breaking changes
- Updated the minimum supported C++ standard to C++17

## 0.28.0 - 2025-01-21

### Breaking changes
- Updated enumerations for unreleased datasets and publishers.

### Enhancements
- Added new dataset `EQUS.MINI` and new publishers `EQUS.MINI.EQUS`, `XNYS.TRADES.EQUS`

### Bug fixes
- Changed historical metadata methods with `symbols` parameter to use a `POST` request
  to allow for requesting supported maximum of 2000 symbols

## 0.27.0 - 2025-01-07

### Breaking changes
- Converted the `UserDefinedInstrument` enum class to an enum to safely allow handling
  invalid data and adding future variants
- Updated the value of the `kMaxRecordLen` constant for the changes to
  `InstrumentDefMsg` in version 3

### Enhancements
- Added `v3` namespace in preparation for future DBN version 3 release. DBN version 2
  remains the current and default version
- Added `v3::InstrumentDefMsg` record with new fields to support normalizing multi-leg
  strategy definitions
  - Removal of statistics-schema related fields `trading_reference_price`,
    `trading_reference_date`, and `settl_price_type`
  - Removal of the status-schema related field `md_security_trading_status`

## 0.26.0 - 2024-12-17

### Breaking changes
- Removed deprecated `Packaging` enum and `packaging` field that's no longer supported
  by the API
- Renamed `VersionUpgradePolicy::Upgrade` to `UpgradeToV2` in preparation for a future
  DBN version 3

### Enhancements
- Created separate namespaces for each DBN version to create a consistent way to refer
  to record types from a particular DBN version regardless of whether the record changed
  between versions

## 0.25.0 - 2024-11-12

### Enhancements
- Added new IntelligentCross venues `ASPN`, `ASMT`, and `ASPI`

### Deprecations
- Deprecated `Packaging` enum and `packaging` field on `BatchJob`. These will be
  removed in a future version. All files from a batch job can be downloaded with the
  `BatchDownload` method on the historical client

## 0.24.0 - 2024-10-22

### Enhancements
- Added new `None` `Action` variant that will be gradually rolled out
  to historical and live `GLBX.MDP3` data

## 0.23.0 - 2024-09-25

### Enhancements
- Added new `Cmbp1Msg`
- Added new consolidated publisher values for `XNAS.BASIC` and `DBEQ.MAX`

### Breaking changes
- Changed the layout of `CbboMsg` to better match `BboMsg`
- Renamed `Schema::Cbbo` to `Schema::Cmbp1`

### Deprecations
- Deprecated `Packing::Tar` and renamed it to `TarDeprecated`. This variant will be
  removed in a future version when it is no longer supported by the API

## 0.22.0 - 2024-08-27

### Enhancements
- Added `Intraday` variant to `DatasetCondition` in preparation for intraday data being
  available from the historical API
- Renamed `example` directory to `examples`
- Renamed `test` directory to `tests`
- Added new publisher values for `XCIS.BBOTRADES` and `XNYS.BBOTRADES`

### Breaking changes
- Removed previously deprecated `DatasetCondition::Bad` variant

## 0.21.0 - 2024-07-30

### Enhancements
- Added new publisher value for `DBEQ.SUMMARY`

### Breaking changes
- Renamed `SType::Nasdaq` variant to `SType::NasdaqSymbol`
- Renamed `SType::Cms` variant to `SType::CmsSymbol`

### Bug fixes
- Added missing `ToString` and `FromString` branches for `SType::NasdaqSymbol` and
  `SType::CmsSymbol`
- Removed `has_header_v` variable template that broke C++11 compatibility

## 0.20.1 - 2024-07-16

### Enhancements
- Improved installation with `CMake`: license is now installed, transitive dependencies
  are configured when importing package

## 0.20.0 - 2024-07-09

This release adds support for encoding DBN within the C++ client.
It also improves historical symbology support with the new `TsSymbolMap` class that
handles mapping historical records to a text symbol. To support this class, several types
for date fields were changed from strings or ints to `date::year_month_day`.

### Enhancements
- Added `TsSymbolMap` to support historical symbology where mappings change between days
- Added `DbnEncoder` class for encoding DBN data
- Added blocking API similar to `LiveBlocking` to `DbnFileStore` with new `GetMetadata`
  and `NextRecord` methods
- Added `BboMsg` record struct for future `bbo-1m` and `bbo-1s` schemas
- Added `PitSymbol` map constructor from `Metadata` and a `date::year_month_day`
- Added `Metadata::CreateSymbolMap` and `Metadata::CreateSymbolMapForDate` methods for
  creating symbology maps from historical metadata
- Added blocking API similar to `LiveBlocking` to `DbnFileStore`
- Added `SymbologyResolution::CreateSymbolMap` method for creating a symbology map from
  a symbology resolution response
- Added `InFileStream` and `OutFileStream` helper classes for reading and writing binary
  output respectively

### Breaking changes
- Added new dependency on [Howard Hinnant's date library](https://howardhinnant.github.io/date/date.html)
- Added `ILogReceiver*` parameter to all `DbnDecoder` constructors and one `DbnFileStore` constructor
- Removed type `StrMappingInterval`. `MappingInterval` is now also used in `SymbologyResolution`.
- Changed `Bbo1sMsg` and `Bbo1mMsg` to be aliases for `BboMsg`
- Changed type of `start_date` and `end_date` in `MappingInterval` to `date::year_month_day`
- Added `stype_in` and `stype_out` fields to `SymbologyResolution` to support creating
  a `TsSymbolMap`

## 0.19.1 - 2024-06-25

### Enhancements
- Added `Upgrade()` method to `Metadata` to update the metadata fields according to a
  `VersionUpgradePolicy`
- Added new publisher values for `XNAS.BASIC` and `XNAS.NLS`

### Bug fixes
- Fixed issue where `Metadata` wasn't upgraded when passing
  `VersionUpgradePolicy::Upgrade`

## 0.19.0 - 2024-06-04

### Enhancements
- Added configurable `heartbeat_interval` parameter for live clients that determines the
  timeout before heartbeat `SystemMsg` records will be sent. It can be configured via
  the `SetHeartbeatInterval` method of the `LiveBuilder`
- Added `SetAddress` method to `LiveBuilder` for configuring a custom gateway address
  without using the constructor directly
- Added new `UncrossingPrice` `StatType` variant
- Added new publisher values for `XNAS.BASIC`
- Added `SetDataset(Dataset)` overload to `LiveBuilder`
- Added new off-market publisher values for `IFEU.IMPACT` and `NDEX.IMPACT`

### Breaking changes
- Added `heartbeat_interval` parameter to the `Live` constructors
- Removed `start_date` and `end_date` fields from `DatasetRange` struct
  in favor of `start` and `end`
- Removed live `Subscribe` method overloads with `use_snapshot`
  parameter in favor of separate `SubscribeWithSnapshot` method

### Bug fixes
- Fixed overloading of live `Subscribe` methods
- Fixed live subscribing with default-constructed `UnixNanos`
- Fixed descriptions for `FINN` and `FINY` publishers

## 0.18.1 - 2024-05-22

### Enhancements
- Added live `Subscribe` function overload with `use_snapshot` parameter
- Added `GetIf` method to `Record` that allows `if` chaining for handling multiple
  record types
- Added record type checking to `Record::Get` method to catch programming errors
  and prevent reading invalid data

### Bug fixes
- Added missing symbol chunking for live `Subscribe` overloads with `const std::string&`
  `start` parameter

## 0.18.0 - 2024-05-14

### Breaking changes
- Changed `FlagSet` to be more class-like:
  - Added predicate methods and setters for each bit flag
  - Improved string formatting
  - Removed bitwise operators. Bitwise operations can be performed by first casting to a
    `std::uint8_t` or calling the `Raw()` method
- Changed format of `display_factor` and `price_ratio` to a fixed-precision decimal for
  `InstrumentDefMsg` and `InstrumentDefMsgV1` to match existing values and DBN crate
- Changed format of `unit_of_measure_qty` to a fixed-precision decimal for
  `InstrumentDefMsgV1` to match `InstrumentDefMsg`

## 0.17.1 - 2024-04-08

### Enhancements
- Added support for Conan-installed zstd (credit: @Hailios)

### Bug fixes
- Added missing copying of `ts_event` when upgrading structs from DBNv1 to DBNv2
- Fixed setting of compiler warnings and warnings that had accumulated

## 0.17.0 - 2024-04-01

### Enhancements
- Added `StatusMsg` record, and `StatusAction`, `StatusReason`, `TradingEvent`, and
  `TriState` enums
- Added `CbboMsg` record and corresponding `ConsolidatedBidAskPair` structure
- Added new enum values for `Schema` and `RType` corresponding to new schemas
  `cbbo`, `cbbo-1s`, `cbbo-1m`, `tcbbo`, `bbo-1s`, `bbo-1m`
- Added `Volatility` and `Delta` `StatType` variants
- Added `Undefined` and `TimeProRata` `MatchAlgorithm` variants
- Changed format of `unit_of_measure_qty` to a fixed-precision decimal
- Added logic to skip `find_package` call if `nlohmann_json` and `httplib` targets
  already exist (credit: @akovachev)
- Added specific instructions for installing dependencies on Ubuntu and macOS (credit: @camrongodbout)

### Breaking changes
- Renamed publishers from deprecated datasets to their respective sources (`XNAS.NLS`
  and `XNYS.TRADES` respectively)

### Deprecations
- Deprecated dataset values `FINN.NLS` and `FINY.TRADES`

### Bug fixes
- Fixed out-of-order initialization in `DbnDecoder` (credit: @Hailios)
- Renamed `MatchAlgorithm::EurodollarOptions` to `MatchAlgorithm::EurodollarFutures`

## 0.16.0 - 2024-03-01

### Enhancements
- Added new publisher values for consolidated DBEQ.MAX
- Added constructor to `WithTsOut` that updates `length` to the correct value to account
  for the extra 8 bytes
- Upgraded default cpp-httplib version to 0.14.3 (last to still support OpenSSL 1.1)
- Upgraded default nlohmann_json version to 3.11.3

### Breaking changes
- Changed default `upgrade_policy` to `Upgrade` so by default the primary record types
  can always be used
- Renamed `dummy` field in `ImbalanceMsg` and `StatMsg` to `reserved`

### Bug fixes
- Fixed handling of `ts_out` when decoding DBNv1 and upgrading to version 2
- Fixed missing logic to upgrade `ErrorMsgV1` and `SystemMsgV1` when decoding DBN with
  `VersionUpgradePolicy::Upgrade`
- Added missing `StatType::Vwap` variant used in the ICE datasets
- Added missing `ToString` and `operator<<` handling for `StatType::ClosePrice` and
  `StatType::NetChange`
- Fixed potential for invalid reads when decoding C strings in `DbnDecoder`

## 0.15.0 - 2024-01-16

### Breaking changes

- Increased size of `SystemMsg` and `ErrorMsg` to provide better messages from Live
  gateway
  - Increased length of `err` and `msg` fields for more detailed messages
  - Added `is_last` field to `ErrorMsg` to indicate the last error in a chain
  - Added `code` field to `SystemMsg` and `ErrorMsg`, although currently unused
  - Added new `is_last` parameter to `ErrorMsg::new`
  - Decoding these is backwards-compatible and records with longer messages won't be
    sent during the DBN version 2 migration period
  - Renamed previous records to `ErrorMsgV1` and `SystemMsgV1`

## 0.14.1 - 2023-12-18

### Enhancements

- Added `PitSymbolMap` helper for keeping track of symbology mappings in Live
- Added new publisher value for OPRA MIAX Sapphire

### Bug fixes

- Fixed misaligned read undefined behavior when decoding records

## 0.14.0 - 2023-11-23

This release adds support for DBN v2.

DBN v2 delivers improvements to the `Metadata` header symbology, new `stype_in` and `stype_out`
fields for `SymbolMappingMsg`, and extends the symbol field length for `SymbolMappingMsg` and
`InstrumentDefMsg`. The entire change notes are available [here](https://github.com/databento/dbn/releases/tag/v0.14.0).
Users who wish to convert DBN v1 files to v2 can use the `dbn-cli` tool available in the [databento-dbn](https://github.com/databento/dbn/) crate.
On a future date, the Databento live and historical APIs will stop serving DBN v1.

This release is fully compatible with both DBN v1 and v2, and so the change should be
seamless for most users.

### Enhancements

- Added support for DBN encoding version 2 (DBNv2), affecting `SymbolMappingMsg`,
  `InstrumentDefMsg`, and `Metadata`
  - Version 1 structs can be converted to version 2 structs with the `ToV2()` method
- Added `symbol_cstr_len` field to `Metadata` to indicate the length of fixed symbol
  strings
- Added `stype_in` and `stype_out` fields to `SymbolMappingMsg` to provide more context
  with live symbology updates
- Added `IndexTs` methods to every record type which returns the primary timestamp
- Added `VersionUpgradePolicy` enum to allow specifying how to handle decoding records
  from prior DBN versions
- Added `InstrumentDefMsgV2` and `SymbolMappingMsgV2` type aliases
- Added `kDbnVersion` constant for current DBN version
- Added `kSymbolCstrLen`, `kSymbolCstrLenV1`, and `kSymbolCstrLenV2` constants for the
  length of fixed-length symbol strings in different DBN versions
- Added new publisher values in preparation for IFEU.IMPACT and NDEX.IMPACT datasets
- Added new publisher values for consolidated DBEQ.BASIC and DBEQ.PLUS
- Added `kMaxRecordLen` constant for the length of the largest record type
- Added ability to convert `FlagSet` to underlying representation

### Breaking changes

- The old `InstrumentDefMsg` is now `InstrumentDefMsgV1` in `compat.hpp`
- The old `SymbolMappingMsg` is now `SymbolMappingMsgV1` in `compat.hpp`
- Converted the following enum classes to enums to allow safely adding new variants:
  `SecurityUpdateAction` and `SType`
- Renamed `dummy` to `reserved` in `InstrumentDefMsg`
- Removed `reserved2`, `reserved3`, `reserved4`, and `reserved5` from `InstrumentDefMsg`
- Moved position of `strike_price` within `InstrumentDefMsg`
- Removed deprecated `SecurityUpdateAction::Invalid` variant

## 0.13.1 - 2023-10-23

### Enhancements

- Added new publisher values in preparation for DBEQ.PLUS
- Added `ToIso8601` for `UnixNanos` for converting to human-readable ISO8601 datetime
  string
- Added `kUndefTimestamp` and `kUndefStatQuantity` constants
- Added flag `kTob` for top-of-book messages

## 0.13.0 - 2023-09-21

### Enhancements

- Added `pretty_px` option for `BatchSubmitJob`, which formats prices to the correct
  scale using the fixed-precision scalar 1e-9 (available for CSV and JSON text
  encodings)
- Added `pretty_ts` option for `BatchSubmitJob`, which formats timestamps as ISO 8601
  strings (available for CSV and JSON text encodings)
- Added `map_symbols` option to `BatchSubmitJob`, which appends the raw symbol
  to every record (available for CSV and JSON text encodings) reducing the need to look
  at the `symbology.json` file
- Added `split_symbols` option for `BatchSubmitJob`, which will split files by raw symbol
- Added `encoding` option to `BatchSubmitJob` to allow requesting non-DBN encoded
  data through the client
- Added `map_symbols`, `pretty_px`, and `pretty_ts` to `BatchJob` response
- Added `ARCX.PILLAR.ARCX` publisher
- Added `ClosePrice` and `NetChange` `StatType`s used in the `OPRA.PILLAR` dataset

### Breaking changes

- Remove `default_value` parameter from `Historical::SymbologyResolve`

## 0.12.1 - 2023-08-25

### Bug fixes

- Fixed typo in `BATY.PITCH.BATY` publisher

## 0.12.0 - 2023-08-24

##### Enhancements

- Added the `Publisher`, `Venue`, and `Dataset` enums
- Added `Publisher` getters to `Record` and `RecordHeader` to convert the
  `publisher_id` to its enum

## 0.11.0 - 2023-08-10

#### Enhancements

- Added `raw_instrument_id` to definition schema
- Added `operator==` and `operator!=` implementations for `DatasetConditionDetail` and
  `DatasetRange`

#### Breaking changes

- Changed `MetadataListPublishers` to return a `vector<PublisherDetail>`
- `MetadataListFields`:
  - Changed return type to `vector<FieldDetail>`
  - Made `encoding` and `schema` parameters required
  - Removed `dataset` parameter
- `MetadataListUnitPrices`:
  - Changed return type to `vector<UnitPricesForMode>`
  - Made `dataset` parameter required
  - Removed `mode` and `schema` parameters

#### Bug fixes

- Fixed installation of `nlohmann_json` when using bundled version
- Added missing `operator!=` implementations for `Metadata`, `MappingInterval`, and
  `SymbolMapping`

## 0.10.0 - 2023-07-20

#### Enhancements

- Added preliminary support for Windows
- Added `LiveThreaded::BlockForStop` to make it easier to wait for one or more records
  before closing the session
- Changed `TimeseriesGetRange` to request a Zstd-compressed result for more efficient
  data transfer
- Switched `BatchSubmitJob` to use form data to avoid query param length limit
- Switched `SymbologyResolve` to use POST request with form data to avoid query param
  length limit

#### Breaking changes

- Changed size-related fields and `limit` parameters to use `std::uint64_t` for consistency
  across architectures

#### Bug fixes

- Removed usage of non-portable `__PRETTY_FUNCTION__`

## 0.9.1 - 2023-07-11

#### Enhancements

- Added constants for dataset codes for Databento Equity Basic and OPRA Pillar
- Added `const char*` getters to records for fixed-length `char` arrays
- Added `RType` getter to `Record`

#### Bug fixes

- Added batching for live subscriptions to avoid hitting max message length
- Fixed bug in Zstd decompression
- Fixed `Historical::BatchDownload` truncating file before writing each chunk

## 0.9.0 - 2023-06-13

#### Enhancements

- Added `Reconnect` methods to `LiveBlocking` and `LiveThreaded`
- Added optional `exception_callback` argument to `LiveThreaded::Start` to improve
  error handling options
- Added batch download support data files (`condition.json` and `symbology.json`)
- Added support for logging warnings from Historical API
- Relaxed 10 minute minimum request time range restriction

#### Breaking changes

- Changed `use_ts_out` default to `false`

#### Bug fixes

- Fixed missing definition for `operator==` for `ImbalanceMsg`

## 0.8.0 - 2023-05-16

#### Enhancements

- Changed `end` and `end_date` to optional to support new forward-fill behaviour

#### Breaking changes

- Renamed `booklevel` MBP field to `levels` for brevity and consistent naming
- Removed `open_interest_qty` and `cleared_volume` fields from definition schema
  that were always unset

## 0.7.0 - 2023-04-28

#### Enhancements

- Added initial support for live data with `LiveBlocking` and `LiveThreaded` clients
- Added support for statistics schema
- Added `SystemMsg` and `ErrorMsg` records for use in live data
- Added `strike_price`, `strike_price_currency`, and `instrument_class` to
  `InstrumentDefMsg`
- Added `FixedPx` helper class for formatting fixed prices
- Added configurable log receiver `ILogReceiver`
- Added `instrument_class`, `strike_price`, and `strike_price_currency` to definition
  schema
- Added additional `condition` variants for `DatasetConditionDetail` (degraded, pending,
  missing)
- Added additional member `last_modified_date` to `DatasetConditionDetail`
- Added `has_mixed_schema`, `has_mixed_stype_in`, and `ts_out` to `Metadata` to support
  live data
- Added optional `compression` parameter to `BatchSubmitJob`

#### Breaking changes

- Removed `related` and `related_security_id` from `InstrumentDefMsg`
- Renamed `BatchJob.cost` to `cost_usd` and value now expressed as US dollars
- Renamed `SType::ProductId` to `SType::InstrumentId` and `SType::Native` to
  `SType::RawSymbol`
- Renamed `RecordHeader::product_id` to `instrument_id`
- Renamed `InstrumentDefMsg::symbol` to `raw_symbol`
- Renamed `SymbolMapping::native_symbol` to `raw_symbol`
- Changed `expiration` and `action` type to `UnixNanos`
- Changed some fields to enums in `InstrumentDefMsg`

#### Deprecations

- Deprecated `SType::Smart` to split into `SType::Parent` and `SType::Continuous`

#### Bug fixes

- Fixed parsing of `BatchSubmitJob` response
- Fixed invalid read in `DbnDecoder`
- Fixed memory leak in `TryCreateDir`

## 0.6.1 - 2023-03-28

#### Breaking changes

- Removed usage of unreliable `std::ifstream::readsome`

#### Bug fixes

- Fixed Zstd decoding of files with multiple frames

## 0.6.0 - 2023-03-24

#### Enhancements

- Added support for imbalance schema
- Added support for decoding `ts_out` field
- Added flags `kSnapshot` and `kMaybeBadBook`

#### Breaking changes

- Removed `record_count` from `Metadata`
- Changed `Historical::BatchDownload` to return the paths of the downloaded files

## 0.5.0 - 2023-03-13

#### Enhancements

- Added `Historical::MetadataGetDatasetRange`

#### Breaking changes

- Changed `MetadataGetDatasetCondition` to return `vector<DatasetConditionDetail>`
- Removed `MetadataListCompressions` (redundant with docs)
- Removed `MetadataListEncodings` (redundant with docs)
- Removed optional `start` and `end` params from `MetadataListSchemas` (redundant)
- Renamed `FileBento` to `DbnFileStore`

## 0.4.0 - 2023-03-02

#### Enhancements

- Added live gateway resolution
- Added `SymbolMappingMsg` and `ErrorMsg` records
- Added `Action` and `Side` enums
- Added `available_start_date` and `available_end_date` to
  `DatasetConditionInfo`
- Made `start_date` and `end_date` optional for
  `Historical::MetadataGetDatasetCondition`
- Improved API for `flags` record fields
- Added `PKGBUILD` to demonstrate installation
- Disabled unit testing by default

#### Breaking changes

- Removed `is_full_universe` and `is_example` fields from `BatchJob`
- Refactored rtypes
  - Introduced separate rtypes for each OHLCV schema
- Renamed DBZ to DBN
  - Renamed `DbzParser` to `DbnDecoder`
- Renamed `TimeseriesStream` to `TimeseriesGetRange`
- Changed `kAllSymbols` representation

#### Bug fixes

- Fixed usage of as a system library

## 0.3.0 - 2023-01-06

#### Enhancements

- Added support for definition schema
- Added option for CMake to download gtest
- Updated `Flag` enum

#### Breaking changes

- Standardized getter method names to pascal case
- Renamed `is_full_book` to `is_full_universe`
- Renamed `TickMsg` to `MboMsg`
- Changed `flags` fields to unsigned

#### Bug fixes

- Fixed cancellation in `Historical::TimeseriesStream`
- Fixed race condition in `Historical::TimeseriesStream` exception handling
- Fixed gtest linker error on macOS

## 0.2.0 - 2022-12-01

#### Enhancements

- Added `Historical::MetadataGetDatasetCondition`
- Improved Zstd CMake integration

#### Bug fixes

- Fixed requesting all symbols for a dataset

## 0.1.0 - 2022-11-07

- Initial release with support for historical data
