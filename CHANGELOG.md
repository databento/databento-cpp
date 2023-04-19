# Changelog

## 0.7.0 - TBD
- Added initial support for live data with `LiveBlocking` and `LiveThreaded` clients
- Added `SystemMsg` and `ErrorMsg` records for use in live data
- Added `strike_price`, `strike_price_currency`, and `instrument_class` to `InstrumentDefMsg`
- Renamed `BatchJob.cost` to `cost_usd` and value now expressed as US dollars
- Added `instrument_class`, `strike_price`, and `strike_price_currency` to definition schema
- Added additional `condition` variants for `DatasetConditionDetail` (degraded, pending, missing)
- Added additional member `last_modified_date` to `DatasetConditionDetail` Added `has_mixed_schema`, `has_mixed_stype_in`, and `ts_out` to `Metadata` to support live data
- Removed `related` and `related_security_id` from `InstrumentDefMsg`
- Changed some fields to enums in `InstrumentDefMsg`
- Added optional `compression` parameter to `BatchSubmitJob`
- Fixed parsing of `BatchSubmitJob` response
- Fixed invalid read in `DbnDecoder`
- Fixed memory leak in `TryCreateDir`

## 0.6.1 - 2023-03-28
- Fixed Zstd decoding of files with multiple frames
- Removed usage of unreliable `std::ifstream::readsome`

## 0.6.0 - 2023-03-24
- Added support for imbalance schema
- Added support for decoding `ts_out` field
- Removed `record_count` from `Metadata`
- Changed `Historical::BatchDownload` to return the paths of the downloaded files
- Added flags `kSnapshot` and `kMaybeBadBook`

## 0.5.0 - 2023-03-13
- Added `Historical::MetadataGetDatasetRange`
- Changed `MetadataGetDatasetCondition` to return `vector<DatasetConditionDetail>`
- Removed `MetadataListCompressions` (redundant with docs)
- Removed `MetadataListEncodings` (redundant with docs)
- Removed optional `start` and `end` params from `MetadataListSchemas` (redundant)
- Renamed `FileBento` to `DbnFileStore`

## 0.4.0 - 2023-03-02
- Renamed DBZ to DBN
  - Renamed `DbzParser` to `DbnDecoder`
- Renamed `TimeseriesStream` to `TimeseriesGetRange`
- Refactored rtypes
  - Introduced separate rtypes for each OHLCV schema
- Added live gateway resolution
- Added `SymbolMappingMsg` and `ErrorMsg` records
- Improved API for `flags` record fields
- Added `Action` and `Side` enums
- Changed `kAllSymbols` representation
- Fixed usage of as a system library
- Removed `is_full_universe` and `is_example` fields from `BatchJob`
- Disabled unit testing by default
- Added `PKGBUILD` to demonstrate installation
- Made `start_date` and `end_date` optional for
  `Historical::MetadataGetDatasetCondition`
- Added `available_start_date` and `available_end_date` to
  `DatasetConditionInfo`

## 0.3.0 - 2023-01-06
- Added support for definition schema
- Fixed cancellation in `Historical::TimeseriesStream`
- Fixed race condition in `Historical::TimeseriesStream` exception handling
- Fixed gtest linker error on macOS
- Added option for CMake to download gtest
- Renamed `TickMsg` to `MboMsg`
- Changed `flags` fields to unsigned
- Updated `Flag` enum
- Standardized getter method names to pascal case
- Renamed `is_full_book` to `is_full_universe`

## 0.2.0 - 2022-12-01
- Added `Historical::MetadataGetDatasetCondition`
- Improved Zstd CMake integration
- Fixed requesting all symbols for a dataset

## 0.1.0 - 2022-11-07
- Initial release with support for historical data
