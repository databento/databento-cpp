#pragma once

#include <locale>
#include <string>

namespace databento {
class ScopedPeriodSeparatorLocale {
 public:
  ScopedPeriodSeparatorLocale()
      : prev_{std::locale::global(
            std::locale{std::locale::classic(), new PeriodSeparator})} {}
  ScopedPeriodSeparatorLocale(const ScopedPeriodSeparatorLocale&) = delete;
  ScopedPeriodSeparatorLocale& operator=(const ScopedPeriodSeparatorLocale&) = delete;
  ScopedPeriodSeparatorLocale(ScopedPeriodSeparatorLocale&&) = delete;
  ScopedPeriodSeparatorLocale& operator=(ScopedPeriodSeparatorLocale&&) = delete;
  ~ScopedPeriodSeparatorLocale() { std::locale::global(prev_); }

 private:
  struct PeriodSeparator : std::numpunct<char> {
    std::string do_grouping() const override { return "\3"; }
    char do_thousands_sep() const override { return '.'; }
  };

  std::locale prev_;
};
}  // namespace databento
