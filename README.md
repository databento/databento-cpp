# databento-cpp

[![test](https://github.com/databento/databento-cpp/actions/workflows/build.yaml/badge.svg?branch=main)](https://github.com/databento/databento-cpp/actions/workflows/build.yaml)
![license](https://img.shields.io/github/license/databento/databento-cpp?color=blue)

The official C++ client library for [Databento](https://databento.com).
The client supports both streaming live and historical data through similar interfaces.

**Please note:** this client currently only supports historical data and is under active development as Databento prepares to launch live data.

## Usage

A simple program that fetches a days worth of historical trades for all ES symbols and prints it looks like this:

```cpp
#include <databento/historical.hpp>
#include <iostream>

using namespace databento;

static constexpr auto kApiKey = "YOUR_API_KEY";

int main() {
  auto client = HistoricalBuilder{}.SetKey(kApiKey).Build();
  client.TimeseriesStream("GLBX.MDP3", "2022-06-10", "2022-06-11", {"ES"},
                          Schema::Trades, SType::Smart, SType::ProductId, {},
                          {}, [](const Record& record) {
                            const auto& trade_msg = record.get<TradeMsg>();
                            std::cout << trade_msg << '\n';
                            return KeepGoing::Continue;
                          });
}
```

To run this program, replace `YOUR_API_KEY` with an actual API key.

Additional example standalone executables are provided in the [examples](./examples) directory.
These examples can be compiled by enabling the cmake option `DATABENTO_ENABLE_EXAMPLES` with `-DDATABENTO_ENABLE_EXAMPLES=1` during the configure step.

### Documentation

More detailed examples and the full API documentation can be found on the [Databento doc site](https://docs.databento.com/getting-started).

## Integration

databento-cpp can be integrated into C++ projects in a couple of different ways.

### Embedded

The easiest way to integrate databento-cpp into your project is using CMake's [FetchContent](https://cmake.org/cmake/help/latest/module/FetchContent.html).
The minimum supported CMake version is 3.14.
```cmake
include(FetchContent)

FetchContent_Declare(
  databento
  GIT_REPOSITORY https://github.com/databento/databento-cpp
  GIT_TAG HEAD
)
FetchContent_MakeAvailable(databento)

add_library(my_library)
target_link_libraries(my_library PRIVATE databento::databento)
```

### System

To install databento-cpp at the system level, clone the repo, build, and install with CMake.
```sh
git clone https://github.com/databento/databento-cpp
cd databento-cpp
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCMAKE_INSTALL_PREFIX='/usr'
cmake --build build --target databento
cmake --install build
```

Then in your project's `CMakeLists.txt`, add the following:
```cmake
find_package(databento 0.1.0 REQUIRED)
add_library(my_library)
target_link_libraries(my_library PRIVATE databento::databento)
```

## Requirements

The minimum C++ standard is C++11 and CMake 3.14.
The library has the following dependencies:
- [cpp-httplib (header only)](https://github.com/yhirose/cpp-httplib)
  - OpenSSL
- [nlohmann_json (header only)](https://github.com/nlohmann/json)
- [Zstandard (zstd)](https://github.com/facebook/zstd)

By default, cpp-httplib and nlohmann_json are downloaded by CMake as part of the build process.
If you would like to use a local version of these libraries, enable the CMake flag
`DATABENTO_ENABLE_EXTERNAL_HTTPLIB` or `DATABENTO_ENABLE_EXTERNAL_JSON`.

The other two dependencies (zstd and OpenSSL) are available in most package managers.
For example, on Ubuntu and Debian:
```sh
sudo apt install libssl-dev libzstd-dev
```

## Building locally

databento-cpp uses [CMake](https://cmake.org/) as its build system, with a minimum version of 3.14.
Building with `cmake` is a two step process: first configuring, then building.
```sh
cmake -S . -B build  # configure
cmake --build build -- -j $(nproc)  # build all targets with all cores
```

### Testing

Tests are located in the `test` directory.
They're written using [GoogleTest (gtest)](https://github.com/google/googletest).
The test target is `databentoTests` and can be build and run with the following commands:
```sh
cmake -S . -B build  # configure
cmake --build build --target databentoTests  # build
build/test/databentoTests  # run
```

### Formatting

databento-cpp uses [`clang-format`](https://clang.llvm.org/docs/ClangFormat.html) with Google's style.
`clang-format` usually comes installed with [clang](https://clang.llvm.org/).
You can run `clang-format` against all the files in `databento-cpp` with the following command:
```sh
cmake --build build --target clang-format
```

### Linting

databento-cpp uses Clang-Tidy and Cppcheck for linting and detecting potential mistakes.
Both can be enabled to run as part of compilation through CMake flags.
```sh
cmake -S . -B build -DDATABENTO_ENABLE_CLANG_TIDY=1 -DDATABENTO_ENABLE_CPPCHECK=1
cmake --build build   # compiles code, and runs Clang-Tidy and Cppcheck
```

### macOS

To setup OpenSSL and zstd, run the following:
```sh
brew install openssl zstd
# Add it to the PATH so cmake can find it
export "$PATH:$HOMEBREW_PREFIX/opt/openssl/bin"
```

For linting on macOS, the best way to install clang-tidy and clang-format is to install all of LLVM
and symlink the binaries to some place in your `PATH`.
```sh
brew install llvm
ln -s $(brew --prefix llvm)/bin/clang-tidy $HOME/.local/bin/
ln -s $(brew --prefix llvm)/bin/clang-format $HOME/.local/bin/
ln -s $(brew --prefix llvm)/bin/run-clang-format $HOME/.local/bin/
```

## License

Distributed under the [Apache 2.0 License](https://www.apache.org/licenses/LICENSE-2.0.html).
