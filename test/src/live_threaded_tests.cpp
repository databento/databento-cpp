#include <gtest/gtest.h>

#include <atomic>
#include <cstdint>

#include "databento/datetime.hpp"
#include "databento/live_threaded.hpp"
#include "databento/record.hpp"
#include "mock/mock_lsg_server.hpp"

namespace databento {
namespace test {
class LiveThreadedTests : public testing::Test {
 protected:
  static constexpr auto kKey = "32-character-with-lots-of-filler";
};

TEST_F(LiveThreadedTests, TestBasic) {
  const mock::MockLsgServer mock_server{[](mock::MockLsgServer& self) {
    self.Accept();
    self.Authenticate();
    self.Start();
    self.SendRecord(
        TickMsg{{sizeof(TickMsg), TickMsg::kTypeId, 1, 2, UnixNanos{}},
                1,
                2,
                3,
                0,
                4,
                'A',
                'B',
                UnixNanos{},
                TimeDeltaNanos{},
                100});
  }};

  LiveThreaded target{kKey, "127.0.0.1", mock_server.Port(), false};
  std::atomic<std::uint32_t> call_count{};
  target.Start([&call_count](const Record& rec) {
    ++call_count;
    EXPECT_TRUE(rec.Holds<TickMsg>());
  });
}
}  // namespace test
}  // namespace databento
