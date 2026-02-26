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

TEST(V1Tests, TestErrorMsgCodeUpgrade) {
  v1::ErrorMsg target{RecordHeader{
      sizeof(v1::ErrorMsg) / RecordHeader::kLengthMultiplier, RType::Error, 0, 0, {}}};
  std::strcpy(target.err.data(), "User or API key deactivated");
  const auto res1 = target.ToV2();
  EXPECT_EQ(res1.code, ErrorCode::ApiKeyDeactivated);

  target.err = {};
  std::strcpy(target.err.data(), "User has reached their open connection limit");
  const auto res2 = target.ToV2();
  EXPECT_EQ(res2.code, ErrorCode::ConnectionLimitExceeded);

  target.err = {};
  std::strcpy(target.err.data(), "Failed to resolve symbol: AAPL");
  const auto res3 = target.ToV2();
  EXPECT_EQ(res3.code, ErrorCode::SymbolResolutionFailed);

  target.err = {};
  std::strcpy(target.err.data(), "Internal error");
  const auto res4 = target.ToV2();
  EXPECT_EQ(res4.code, ErrorCode::InternalError);

  target.err = {};
  std::strcpy(target.err.data(), "Slow client detected for mbo. Skipped records");
  const auto res5 = target.ToV2();
  EXPECT_EQ(res5.code, ErrorCode::SkippedRecordsAfterSlowReading);
}
}  // namespace databento::v1::tests
