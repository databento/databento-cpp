# Changelog

## 0.3.0 - 2023-01-06
- Add support for definition schema
- Fix cancellation in `Historical::TimeseriesStream`
- Fix race condition in `Historical::TimeseriesStream` exception handling
- Fix gtest linker error on macOS
- Add option for CMake to download gtest
- Rename `TickMsg` to `MboMsg`
- Change `flags` fields to unsigned
- Update `Flag` enum
- Standardize getter method names to pascal case
- Rename `is_full_book` to `is_full_universe`

## 0.2.0 - 2022-12-01
- Add dataset condition endpoint
- Improve Zstd CMake integration
- Fix requesting all symbols for a dataset

## 0.1.0 - 2022-11-07
- Initial release with support for historical data
