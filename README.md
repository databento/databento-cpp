# databento-cpp

[![test](https://github.com/databento/databento-cpp/actions/workflows/build.yaml/badge.svg?branch=main)](https://github.com/databento/databento-cpp/actions/workflows/build.yaml)
[![license](https://img.shields.io/github/license/databento/databento-cpp?color=blue)](./LICENSE)
[![Slack](https://img.shields.io/badge/join_Slack-community-darkblue.svg?logo=slack)](https://to.dbn.to/slack)

The official C++ client library for [Databento](https://databento.com).
The client supports both streaming real-time and historical market data through similar interfaces.

## Usage

The minimum C++ standard is C++17 and the minimum CMake version is 3.14.

### Integration

The easiest way to use our library is by embedding it with [CMake FetchContent](https://cmake.org/cmake/help/latest/module/FetchContent.html).
Your `CMakeLists.txt` should look something like the following:

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.14)

project(databento_example)
include(FetchContent)

FetchContent_Declare(
  databento
  GIT_REPOSITORY https://github.com/databento/databento-cpp
  GIT_TAG HEAD
)
FetchContent_MakeAvailable(databento)

add_executable(example main.cpp)
target_link_libraries(example PRIVATE databento::databento)
```

Alternatively, you can clone the source code from GitHub [here](https://github.com/databento/databento-cpp).

To install the library to `/usr`, build and install it with the following:

```sh
git clone https://github.com/databento/databento-cpp
cd databento-cpp
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCMAKE_INSTALL_PREFIX='/usr'
cmake --build build --target databento --parallel
cmake --install build
```

In your project's `CMakeLists.txt`, add the following:

```cmake
# CMakeLists.txt
find_package(databento REQUIRED)
target_link_libraries(example PRIVATE databento::databento)
```

### Dependencies

You'll need to ensure the following dependencies are installed:
- [OpenSSL](https://www.openssl.org/) (minimum version 3.0)
- [Libcrypto](https://www.openssl.org/docs/man3.0/man7/crypto.html)
- [Zstandard (zstd)](https://github.com/facebook/zstd)
- [nlohmann\_json (header-only)](https://github.com/nlohmann/json)
- [cpp-httplib (header-only)](https://github.com/yhirose/cpp-httplib)
- [date (header-only)](https://github.com/HowardHinnant/date)

By default, date, cpp-httplib and nlohmann\_json are downloaded by CMake as part of the build process.
If you would like to use a local version of these libraries, enable the CMake flag
`DATABENTO_USE_EXTERNAL_DATE`, `DATABENTO_USE_EXTERNAL_HTTPLIB`, or `DATABENTO_USE_EXTERNAL_JSON` respectively.

#### Ubuntu

Run the following commands to install the dependencies on Ubuntu:
```sh
$ sudo apt update
$ sudo apt install libssl-dev libzstd-dev
```

#### macOS

On macOS, you can install the dependencies with [Homebrew](https://brew.sh/) by running the following:
```sh
$ brew install openssl@3 zstd
```

### Live

Real-time and intraday replay is provided through the Live clients.
Here is a simple program that fetches 10 seconds of trades for all ES mini futures:

```cpp
#include <chrono>
#include <databento/live.hpp>
#include <databento/symbol_map.hpp>
#include <iostream>
#include <string>
#include <thread>

using namespace databento;

int main() {
  PitSymbolMap symbol_mappings;

  auto client =
      LiveBuilder{}.SetKeyFromEnv().SetDataset("GLBX.MDP3").BuildThreaded();

  auto handler = [&symbol_mappings](const Record& rec) {
    symbol_mappings.OnRecord(rec);
    if (const auto* trade = rec.GetIf<TradeMsg>()) {
      std::cout << "Received trade for "
                << symbol_mappings[trade->hd.instrument_id] << ':' << *trade
                << '\n';
    }
    return KeepGoing::Continue;
  };

  client.Subscribe({"ES.FUT"}, Schema::Trades, SType::Parent);
  client.Start(handler);
  std::this_thread::sleep_for(std::chrono::seconds{10});
  return 0;
}
```
To run this program, set the `DATABENTO_API_KEY` environment variable with an actual API key.

### Historical

Here is a simple program that fetches 10 minutes worth of historical trades for two CME futures:

```cpp
#include <databento/dbn.hpp>
#include <databento/historical.hpp>
#include <databento/symbol_map.hpp>
#include <iostream>

using namespace databento;

int main() {
  auto client = HistoricalBuilder{}.SetKey("$YOUR_API_KEY").Build();
  TsSymbolMap symbol_map;
  auto decode_symbols = [&symbol_map](const Metadata& metadata) {
    symbol_map = metadata.CreateSymbolMap();
  };
  auto print_trades = [&symbol_map](const Record& record) {
    const auto& trade_msg = record.Get<TradeMsg>();
    std::cout << "Received trade for " << symbol_map.At(trade_msg) << ": "
              << trade_msg << '\n';
    return KeepGoing::Continue;
  };
  client.TimeseriesGetRange(
      "GLBX.MDP3", {"2022-06-10T14:30", "2022-06-10T14:40"}, kAllSymbols,
      Schema::Trades, SType::RawSymbol, SType::InstrumentId, {}, decode_symbols,
      print_trades);
}
```

To run this program, set the `DATABENTO_API_KEY` environment variable with an actual API key.

Additional example standalone executables are provided in the [`example`](./example) directory.
These examples can be compiled by enabling the cmake option `DATABENTO_ENABLE_EXAMPLES` with `-DDATABENTO_ENABLE_EXAMPLES=1` during the configure step.

## Documentation

You can find more detailed examples and the full API documentation on the [Databento doc site](https://databento.com/docs/quickstart?historical=cpp&live=cpp).

## License

Distributed under the [Apache 2.0 License](https://www.apache.org/licenses/LICENSE-2.0.html).
