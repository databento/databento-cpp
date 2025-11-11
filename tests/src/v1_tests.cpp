#include <gtest/gtest.h>

#include <cstring>  // strcpy

#include "databento/enums.hpp"
#include "databento/record.hpp"
#include "databento/v1.hpp"

namespace databento::v1::tests {
TEST(V1Tests, TestSystemMsgCodeUpgrade) {
  v1::SystemMsg target{
      RecordHeader{sizeof(v1::SystemMsg) / RecordHeader::kLengthMultiplier,
                   RType::System,
                   0,
                   0,
                   {}}};
  std::strcpy(target.msg.data(), "Heartbeat");
  const auto res1 = target.ToV2();
  EXPECT_EQ(res1.code, SystemCode::Heartbeat);

  target.msg = {};
  std::strcpy(target.msg.data(), "End of interval for bbo-1s");
  const auto res2 = target.ToV2();
  EXPECT_EQ(res2.code, SystemCode::EndOfInterval);

  target.msg = {};
  std::strcpy(target.msg.data(), "Subscription request 5 for mbo data succeeded");
  const auto res3 = target.ToV2();
  EXPECT_EQ(res3.code, SystemCode::SubscriptionAck);

  target.msg = {};
  std::strcpy(target.msg.data(),
              "Warning: slow reading, not keeping pace with cbbo-1s data");
  const auto res4 = target.ToV2();
  EXPECT_EQ(res4.code, SystemCode::SlowReaderWarning);

  target.msg = {};
  std::strcpy(target.msg.data(), "Finished ohlcv-1s replay");
  const auto res5 = target.ToV2();
  EXPECT_EQ(res5.code, SystemCode::ReplayCompleted);
}
}  // namespace databento::v1::tests
