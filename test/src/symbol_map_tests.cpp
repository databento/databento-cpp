#include <gtest/gtest.h>

#include <cstdint>
#include <cstring>
#include <string>
#include <unordered_map>

#include "databento/compat.hpp"
#include "databento/record.hpp"
#include "databento/symbol_map.hpp"

namespace databento {
namespace test {
template <typename SM>
SM GenMapping(std::uint32_t instrument_id, const char* stype_out_symbol) {
  SM res = {};
  res.hd = RecordHeader{sizeof(SM) / RecordHeader::kLengthMultiplier,
                        RType::SymbolMapping,
                        1,
                        instrument_id,
                        {}};
  std::strncpy(res.stype_out_symbol.data(), stype_out_symbol,
               res.stype_out_symbol.size());
  return res;
}

TEST(PitSymbolMapTests, TestOnSymbolMapping) {
  PitSymbolMap target;
  target.OnSymbolMapping(GenMapping<SymbolMappingMsgV1>(1, "AAPL"));
  target.OnSymbolMapping(GenMapping<SymbolMappingMsgV2>(2, "TSLA"));
  target.OnSymbolMapping(GenMapping<SymbolMappingMsgV1>(3, "MSFT"));
  std::unordered_map<std::uint32_t, std::string> exp{
      {1, "AAPL"}, {2, "TSLA"}, {3, "MSFT"}};
  ASSERT_EQ(target.Map(), exp);
  target.OnSymbolMapping(GenMapping<SymbolMappingMsgV1>(10, "AAPL"));
  target.OnSymbolMapping(GenMapping<SymbolMappingMsgV2>(1, "MSFT"));
  ASSERT_EQ(target[1], "MSFT");
}

TEST(PitSymbolMapTests, TestOnRecord) {
  PitSymbolMap target;
  auto sm1 = GenMapping<SymbolMappingMsgV1>(1, "AAPL");
  target.OnRecord(Record{&sm1.hd});
  auto sm2 = GenMapping<SymbolMappingMsgV2>(2, "TSLA");
  target.OnRecord(Record{&sm2.hd});
  sm1 = GenMapping<SymbolMappingMsgV1>(3, "MSFT");
  target.OnRecord(Record{&sm1.hd});
  std::unordered_map<std::uint32_t, std::string> exp{
      {1, "AAPL"}, {2, "TSLA"}, {3, "MSFT"}};
  ASSERT_EQ(target.Map(), exp);
  sm1 = GenMapping<SymbolMappingMsgV1>(10, "AAPL");
  target.OnRecord(Record{&sm1.hd});
  sm2 = GenMapping<SymbolMappingMsgV2>(1, "MSFT");
  target.OnRecord(Record{&sm2.hd});
  ASSERT_EQ(target[1], "MSFT");
}
}  // namespace test
}  // namespace databento
