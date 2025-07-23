#include <chrono>
#include <cstddef>
#include <cstdint>

#include "databento/datetime.hpp"
#include "databento/dbn.hpp"
#include "databento/dbn_encoder.hpp"
#include "databento/dbn_file_store.hpp"
#include "databento/detail/zstd_stream.hpp"
#include "databento/enums.hpp"
#include "databento/file_stream.hpp"
#include "databento/flag_set.hpp"
#include "databento/record.hpp"
#include "databento/v1.hpp"
#include "temp_file.hpp"

namespace databento::tests {
TEST(DbnFileStoreTests, TestDecodeExtended) {
  const auto record_gen = [](const std::size_t i) {
    auto flags = FlagSet{};
    if (i % 2 == 0) {
      flags.SetBadTsRecv();
    }
    Action action;
    switch (i % 5) {
      case 0: {
        action = Action::Add;
        break;
      }
      case 1: {
        action = Action::Modify;
        break;
      }
      case 2: {
        action = Action::Cancel;
        break;
      }
      case 3: {
        action = Action::Trade;
        break;
      }
      case 4:
      default: {
        action = Action::Fill;
      }
    }
    Side side;
    switch (i % 7) {
      case 0:
      case 1:
      case 2: {
        side = Side::Ask;
      }
      case 3:
      case 4:
      case 5: {
        side = Side::Bid;
      }
      case 6:
      default: {
        side = Side::None;
      }
    }
    return MboMsg{
        RecordHeader{sizeof(MboMsg) / RecordHeader::kLengthMultiplier, RType::Mbo,
                     static_cast<std::uint16_t>(Publisher::GlbxMdp3Glbx),
                     static_cast<std::uint32_t>(i),
                     UnixNanos{std::chrono::milliseconds{i}}},
        i % 5,
        25'000'000 + static_cast<std::int64_t>(i),
        static_cast<std::uint32_t>(i) % 1'000,
        flags,
        static_cast<std::uint8_t>(i % 16),
        action,
        side,
        UnixNanos{std::chrono::milliseconds{i} +
                  std::chrono::nanoseconds{10 + i % 100}},
        std::chrono::milliseconds{2 + i % 34},
        static_cast<std::uint32_t>(i) / 2

    };
  };
  TempFile temp_file{std::filesystem::temp_directory_path() /
                     "test_decode_extended.dbn.zst"};

  constexpr auto kExpSize = 100'000;
  {
    OutFileStream out_file{temp_file.Path()};
    detail::ZstdCompressStream stream{&out_file};
    DbnEncoder encoder{Metadata{1,
                                ToString(Dataset::GlbxMdp3),
                                Schema::Mbo,
                                {},
                                {},
                                {},
                                {},
                                {},
                                false,
                                databento::v1::kSymbolCstrLen,
                                std::vector<std::string>(4571)},
                       &stream};
    for (std::size_t i = 0; i < kExpSize; ++i) {
      auto flags = FlagSet{};
      encoder.EncodeRecord(record_gen(i));
    }
  }
  std::size_t count = 0;
  DbnFileStore target{temp_file.Path()};
  while (const auto* rec = target.NextRecord()) {
    const auto* mbo = rec->GetIf<MboMsg>();
    ASSERT_NE(mbo, nullptr) << "Found non-MBO record with hd = " << rec->Header()
                            << " at count = " << count;
    ASSERT_EQ(*mbo, record_gen(count)) << "MboMsg mismatch at count = " << count;
    ++count;
  }
  ASSERT_EQ(count, kExpSize);
}
}  // namespace databento::tests
