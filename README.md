# databento-cpp

[![test](https://github.com/databento/databento-cpp/actions/workflows/build.yaml/badge.svg?branch=main)](https://github.com/databento/databento-cpp/actions/workflows/build.yaml)
![license](https://img.shields.io/github/license/databento/databento-cpp?color=blue)

The official C++ client library for [Databento](https://databento.com).
The client supports both streaming live and historical data through similar interfaces.

**Please note:** this client currently supports historical data and is under active development as Databento prepares to launch live data.

## Usage

A simple application that fetches 10 minutes of historical trades for all ES symbols and prints it look like this:

```cpp
#include <chrono>
#include <ctime>
#include <databento/historical.hpp>
#include <iomanip>
#include <iostream>

using namespace databento;

static constexpr auto kApiKey = "YOUR_API_KEY";

int main() {
  auto client = HistoricalBuilder{}.SetKey(kApiKey).Build();
  client.TimeseriesStream("GLBX.MDP3", "2022-06-10T14:30", "2022-05-10T14:40",
                          {"ES"}, Schema::Trades, SType::Smart,
                          SType::ProductId, {}, {}, [](const Record& record) {
                            const auto& trade_msg = record.get<TradeMsg>();
                            std::cout << trade_msg.hd.product_id << ": "
                                      << trade_msg.size << " @ "
                                      << trade_msg.price << std::endl;
                            return KeepGoing::Continue;
                          });
}
```

To run this program, replace `YOUR_API_KEY` with an actual API key.

Additional example standalone executables are provided in the [examples](./examples) directory.
These examples can be compiled by enabling the cmake option `DATABENTO_ENABLE_EXAMPLES` with `-DDATABENTO_ENABLE_EXAMPLES=1` during the configure step.

### Documentation

More detailed examples and the full API documentation can be found on the [Databento doc site](https://docs.databento.com/getting-started).

## Requirements

The minimum C++ standard is C++11.
Dependencies:
- [cpp-httplib (header only)](https://github.com/yhirose/cpp-httplib)
  - OpenSSL
- [nlohmann_json (header only)](https://github.com/nlohmann/json)
- [Zstandard (zstd)](https://github.com/facebook/zstd)

By default, cpp-httplib and nlohmann_json are downloaded by CMake as part of the build process.
If you would like to use a local version of these libraries, enable the CMake flag
`DATABENTO_ENABLE_EXTERNAL_HTTPLIB` or `DATABENTO_ENABLE_EXTERNAL_JSON`.

## Building

databento-cpp uses [CMake](https://cmake.org/) as its build system, with a minimum version of 3.14.
Building with `cmake` is a two step process: first configuring, then building.
```sh
cmake -S . -B build  # configure
cmake --build build -- -j $(nproc)  # build all targets with all cores
```

## Testing

Tests are located in the `test` directory.
They're written using [GoogleTest (gtest)](https://github.com/google/googletest).
The test target is `databentoTests` and can be build and run with the following commands:
```sh
cmake -S . -B build  # configure
cmake --build build --target databentoTests  # build
build/test/databentoTests  # run
```

## Formatting

databento-cpp uses [`clang-format`](https://clang.llvm.org/docs/ClangFormat.html) with Google's style.
`clang-format` usually comes installed with [clang](https://clang.llvm.org/).
You can run `clang-format` against all the files in `databento-cpp` with the following command:
```sh
cmake --build build --target clang-format
```

## Linting

databento-cpp uses clang-tidy for linting and detecting potential mistakes.
To run clang-tidy, run the following command:
```sh
cmake -S . -B build # configure to create compile_commands.json
run-clang-tidy -p build
```

## macOS

On macOS, the best way to install clang-tidy and clang-format is to install all of LLVM and symlink
the binaries to some place in your `PATH`.
```sh
brew install llvm
ln -s $(brew --prefix llvm)/bin/clang-tidy $HOME/.local/bin/
ln -s $(brew --prefix llvm)/bin/clang-format $HOME/.local/bin/
ln -s $(brew --prefix llvm)/bin/run-clang-format $HOME/.local/bin/
```

To setup OpenSSL, run the following:
```sh
brew install openssl
# Add it to the PATH so cmake can find it
export "$PATH:$HOMEBREW_PREFIX/opt/openssl/bin"
```

## License

Distributed under the [Apache 2.0 License](https://www.apache.org/licenses/LICENSE-2.0.html).
