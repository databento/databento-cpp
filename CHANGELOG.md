# Changelog

## 0.6.0 - TBD
- Added support for imbalance schema
- Added support for decoding `ts_out` field
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
