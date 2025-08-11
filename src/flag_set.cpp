#include "databento/flag_set.hpp"

#include <array>

#include "stream_op_helper.hpp"

namespace databento {
std::ostream& operator<<(std::ostream& stream, FlagSet flag_set) {
  const std::array<std::pair<bool (FlagSet::*)() const, const char*>, 7>
      kFlagsAndNames = {{
          {&FlagSet::IsLast, "LAST"},
          {&FlagSet::IsTob, "TOB"},
          {&FlagSet::IsSnapshot, "SNAPSHOT"},
          {&FlagSet::IsMbp, "MBP"},
          {&FlagSet::IsBadTsRecv, "BAD_TS_RECV"},
          {&FlagSet::IsMaybeBadBook, "MAYBE_BAD_BOOK"},
          {&FlagSet::IsPublisherSpecific, "PUBLISHER_SPECIFIC"},
      }};

  bool has_written_flag = false;
  for (const auto& [flag, name] : kFlagsAndNames) {
    if ((flag_set.*flag)()) {
      if (has_written_flag) {
        stream << " | " << name;
      } else {
        stream << name;
        has_written_flag = true;
      }
    }
  }
  // Cast to uint16_t to avoid being formatted as char
  const auto raw = static_cast<std::uint16_t>(flag_set.Raw());
  if (has_written_flag) {
    stream << " (" << raw << ')';
  } else {
    stream << raw;
  }
  return stream;
}

std::string ToString(FlagSet flags) { return MakeString(flags); }
}  // namespace databento
