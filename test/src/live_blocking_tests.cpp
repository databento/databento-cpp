#include <gtest/gtest.h>
#include <openssl/sha.h>  //  SHA256_DIGEST_LENGTH

#include <chrono>
#include <thread>  // this_thread
#include <vector>

#include "databento/constants.hpp"  // dataset
#include "databento/datetime.hpp"
#include "databento/enums.hpp"  // Schema, SType
#include "databento/live_blocking.hpp"
#include "databento/record.hpp"
#include "mock/mock_lsg_server.hpp"  // MockLsgServer

namespace databento {
namespace test {
class LiveBlockingTests : public testing::Test {
 protected:
  static constexpr auto kKey = "32-character-with-lots-of-filler";
};

TEST_F(LiveBlockingTests, TestAuthentication) {
  const mock::MockLsgServer mock_server{[](mock::MockLsgServer& self) {
    self.Accept();
    self.Authenticate();
  }};

  const LiveBlocking target{kKey, "127.0.0.1", mock_server.Port(), false};
}

TEST_F(LiveBlockingTests, TestStart) {
  const mock::MockLsgServer mock_server{[](mock::MockLsgServer& self) {
    self.Accept();
    self.Authenticate();
    self.Start();
  }};

  LiveBlocking target{kKey, "127.0.0.1", mock_server.Port(), false};
  target.Start();
}

TEST_F(LiveBlockingTests, TestSubscribe) {
  constexpr auto kDataset = dataset::kXnasItch;
  const std::vector<std::string> kSymbols{"MSFT", "TSLA", "QQQ"};
  constexpr auto kSchema = Schema::Ohlcv1M;
  constexpr auto kSType = SType::Native;

  const mock::MockLsgServer mock_server{[&kSymbols](mock::MockLsgServer& self) {
    self.Accept();
    self.Authenticate();
    self.Subscribe(kDataset, kSymbols, kSchema, kSType);
  }};

  LiveBlocking target{kKey, "127.0.0.1", mock_server.Port(), false};
  target.Subscribe(kDataset, kSymbols, kSchema, kSType);
}

TEST_F(LiveBlockingTests, TestNextRecord) {
  constexpr auto kRecCount = 12;
  const mock::MockLsgServer mock_server{[](mock::MockLsgServer& self) {
    self.Accept();
    self.Authenticate();
    const OhlcvMsg rec{
        {sizeof(OhlcvMsg) / 4, OhlcvMsg::kTypeId, 1, 1, UnixNanos{}},
        1,
        2,
        3,
        4,
        5};
    for (size_t i = 0; i < kRecCount; ++i) {
      self.SendRecord(rec);
    }
  }};

  LiveBlocking target{kKey, "127.0.0.1", mock_server.Port(), false};
  for (size_t i = 0; i < kRecCount; ++i) {
    const auto rec = target.NextRecord();
    ASSERT_TRUE(rec.Holds<OhlcvMsg>()) << "Failed on call " << i;
  }
}
}  // namespace test
}  // namespace databento
