#pragma once

#include <cassert>  // assert
#include <cstdio>   // remove
#include <fstream>  // ifstream
#include <string>
#include <utility>  // move

#include "databento/exceptions.hpp"

namespace databento {
// A RAII for creating a file on construction and removing it when the class
// goes out of scope.
class TempFile {
 public:
  explicit TempFile(std::string path) : path_{std::move(path)} {
    std::ifstream f{path_};
    if (f.good()) {
      throw InvalidArgumentError{"TempFile::TempFile", "path",
                                 "path shouldn't already exist"};
    }
  }
  TempFile(const TempFile&) = delete;
  TempFile& operator=(const TempFile&) = delete;
  TempFile(TempFile&&) = default;
  TempFile& operator=(TempFile&&) = default;
  ~TempFile() { assert(std::remove(path_.c_str()) == 0); }

  const std::string& Path() const { return path_; }

 private:
  std::string path_;
};
}  // namespace databento
