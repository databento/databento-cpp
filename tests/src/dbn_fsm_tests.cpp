#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

#include "databento/dbn.hpp"
#include "databento/dbn_encoder.hpp"
#include "databento/detail/buffer.hpp"
#include "databento/detail/dbn_fsm.hpp"
#include "databento/enums.hpp"
#include "databento/record.hpp"

namespace databento::detail::tests {
class DbnFsmTests : public testing::TestWithParam<bool> {
 protected:
  static constexpr std::uint32_t kRecordCount = 14;

  void SetUp() override {
    const Metadata metadata{kDbnVersion,
                            dataset::kGlbxMdp3,
                            {},
                            {},
                            {},
                            0,
                            SType::InstrumentId,
                            SType::InstrumentId,
                            false,
                            kSymbolCstrLen,
                            {},
                            {},
                            {},
                            {}};
    DbnEncoder encoder{metadata, &encoded_};
    record_ends_.push_back(encoded_.ReadCapacity());
    for (std::uint32_t i = 0; i < kRecordCount; ++i) {
      if (i == 6) {
        Mbp1Msg mbp1{};
        mbp1.hd = {
            sizeof(Mbp1Msg) / RecordHeader::kLengthMultiplier, RType::Mbp1, 1, i, {}};
        encoder.EncodeRecord(mbp1);
      } else {
        TradeMsg trade{};
        trade.hd = {
            sizeof(TradeMsg) / RecordHeader::kLengthMultiplier, RType::Mbp0, 1, i, {}};
        encoder.EncodeRecord(trade);
      }
      record_ends_.push_back(encoded_.ReadCapacity());
    }
  }

  const std::byte* Begin() const { return encoded_.ReadBegin(); }
  const std::byte* RecordEnd(std::uint32_t count) const {
    return encoded_.ReadBegin() + record_ends_[count];
  }

  void Refill(const std::byte* begin, const std::byte* end) {
    const auto length = static_cast<std::size_t>(end - begin);
    if (GetParam()) {
      std::size_t space_len{};
      auto* space = target_.Space(&space_len);
      ASSERT_GE(space_len, length);
      std::memcpy(space, begin, length);
      target_.Fill(length);
    } else {
      target_.WriteAll(begin, length);
    }
  }

  void TakeRecord() {
    ASSERT_EQ(target_.Process(), DbnFsm::Status::Record);
    instrument_ids_.push_back(target_.LastRecord().Header().instrument_id);
  }

  void Drain() {
    while (target_.Process() == DbnFsm::Status::Record) {
      instrument_ids_.push_back(target_.LastRecord().Header().instrument_id);
    }
  }

  void StartMidBatch() {
    target_.WriteAll(Begin(), static_cast<std::size_t>(RecordEnd(4) - Begin()));
    ASSERT_EQ(target_.Process(), DbnFsm::Status::Metadata);
    Drain();
    target_.WriteAll(RecordEnd(4),
                     static_cast<std::size_t>(RecordEnd(8) - RecordEnd(4)));
    TakeRecord();
  }

  void ExpectAllRecords() {
    std::vector<std::uint32_t> expected(kRecordCount);
    for (std::uint32_t i = 0; i < kRecordCount; ++i) {
      expected[i] = i;
    }
    ASSERT_EQ(instrument_ids_, expected);
    ASSERT_EQ(target_.UnreadBytes(), 0);
  }

 private:
  Buffer encoded_;
  std::vector<std::size_t> record_ends_;
  // Small enough buffer the refill has to move the buffered records
  DbnFsm target_{VersionUpgradePolicy::AsIs, kMaxRecordLen};
  std::vector<std::uint32_t> instrument_ids_;
};

INSTANTIATE_TEST_SUITE_P(Refill, DbnFsmTests, testing::Values(false, true),
                         [](const testing::TestParamInfo<bool>& info) {
                           return info.param ? "SpaceFill" : "WriteAll";
                         });

TEST_P(DbnFsmTests, TestRefillMidBatchKeepsRecords) {
  StartMidBatch();
  Refill(RecordEnd(8), RecordEnd(kRecordCount));
  Drain();
  ExpectAllRecords();
}

TEST_P(DbnFsmTests, TestSecondRefillMidBatchKeepsRecords) {
  StartMidBatch();
  Refill(RecordEnd(8), RecordEnd(11));
  TakeRecord();
  Refill(RecordEnd(11), RecordEnd(kRecordCount));
  Drain();
  ExpectAllRecords();
}
}  // namespace databento::detail::tests
