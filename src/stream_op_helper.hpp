#pragma once

#include <cstdint>
#include <ios>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>

#include "databento/datetime.hpp"

namespace databento {
template <typename T>
// Helper for types that implement a stream operator
std::string MakeString(const T& val) {
  std::ostringstream ss;
  ss << val;
  return ss.str();
}

class StreamOpHelper {
 private:
  template <typename T>
  void FmtToStream(const T& val) {
    stream_ << val;
  }

  void FmtToStream(const std::string& val) {
    // std::quoted requires C++14
    stream_ << '"' << val << '"';
  }

  void FmtToStream(const bool& val) {
    // otherwise bool is formatted (1|0)
    stream_ << std::boolalpha << val;
  }

  void FmtToStream(const char& val) { stream_ << '\'' << val << '\''; }

  void FmtToStream(const std::uint8_t& val) {
    // otherwise is formatted as a char
    stream_ << static_cast<std::uint16_t>(val);
  }

  void FmtToStream(const std::int8_t& val) {
    // otherwise is formatted as a char
    stream_ << static_cast<std::int16_t>(val);
  }

  void FmtToStream(const UnixNanos& val) { stream_ << ToString(val); }

  void FmtToStream(const TimeDeltaNanos& val) { stream_ << ToString(val); }

  void FmtToStream(const std::ostringstream& val) { stream_ << val.str(); }

 public:
  StreamOpHelper(std::ostream& stream, const std::string& type_name,
                 std::string spacer, std::string indent)
      : stream_{stream},
        spacer_{std::move(spacer)},
        indent_{std::move(indent)} {
    if (type_name.empty()) {
      stream_ << '{';
    } else {
      stream_ << type_name << " {";
    }
  }

  template <typename T>
  StreamOpHelper& AddField(const std::string& field_name, const T& field_val) {
    if (!is_first_) {
      stream_ << ',';
    }
    stream_ << spacer_ << indent_ << field_name << " = ";
    FmtToStream(field_val);
    is_first_ = false;
    return *this;
  }

  template <typename T>
  StreamOpHelper& AddItem(const T& item) {
    if (!is_first_) {
      stream_ << ',';
    }
    stream_ << spacer_ << indent_;
    FmtToStream(item);
    is_first_ = false;
    return *this;
  }

  std::ostream& Finish() {
    if (spacer_.find('\n') == std::string::npos) {
      // no spacing required if empty
      if (!is_first_) {
        stream_ << spacer_;
      }
    } else {
      stream_ << '\n' << indent_;
    }
    stream_ << '}';
    return stream_;
  }

 private:
  std::ostream& stream_;
  std::string spacer_;
  std::string indent_;
  bool is_first_{true};
};

class StreamOpBuilder {
 public:
  explicit StreamOpBuilder(std::ostream& stream) : stream_{stream} {}

  StreamOpBuilder& SetTypeName(std::string type_name) {
    type_name_ = std::move(type_name);
    return *this;
  }

  // Sets what's inserted between the comma and the next element
  StreamOpBuilder& SetSpacer(std::string spacer) {
    spacer_ = std::move(spacer);
    return *this;
  }

  // Sets any indentation that should be applied to all elements including the
  // closing '}'. Primarily used for nested structures.
  StreamOpBuilder& SetIndent(std::string indent) {
    indent_ = std::move(indent);
    return *this;
  }

  // Instantiate a `StreamOpHelper` with the current settings.
  StreamOpHelper Build() {
    return StreamOpHelper{stream_, type_name_, spacer_, indent_};
  }

 private:
  std::ostream& stream_;
  std::string indent_;
  std::string type_name_;
  std::string spacer_;
};
}  // namespace databento
