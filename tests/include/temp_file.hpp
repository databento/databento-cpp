#pragma once

#include <gtest/gtest.h>  // EXPECT_EQ

#include <cassert>  // assert
#include <filesystem>
#include <fstream>  // ifstream
#include <string>
#include <utility>  // move

#include "databento/exceptions.hpp"

namespace databento {
// A RAII for creating a file on construction and removing it when the class
// goes out of scope.
class TempFile {
 public:
  explicit TempFile(std::filesystem::path path) : path_{std::move(path)} {
    std::ifstream f{path_};
    if (Exists()) {
      throw InvalidArgumentError{
          "TempFile::TempFile", "path",
          "File at path " + path_.string() + " shouldn't already exist"};
    }
  }
  TempFile(const TempFile&) = delete;
  TempFile& operator=(const TempFile&) = delete;
  TempFile(TempFile&&) = default;
  TempFile& operator=(TempFile&&) = default;
  ~TempFile() {
    const bool ret = std::filesystem::remove(path_);
    EXPECT_TRUE(ret) << "TempFile at " << path_ << " did not exist";
  }

  const std::filesystem::path& Path() const { return path_; }

  bool Exists() const {
    std::ifstream f{path_};
    return f.good();
  }

 private:
  std::filesystem::path path_;
};
}  // namespace databento
