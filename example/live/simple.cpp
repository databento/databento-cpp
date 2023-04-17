#include <csignal>  // sig_atomic_t
#include <cstdint>
#include <iostream>
#include <unordered_map>

#include "databento/constants.hpp"
#include "databento/dbn.hpp"
#include "databento/enums.hpp"
#include "databento/live.hpp"
#include "databento/live_threaded.hpp"
#include "databento/record.hpp"
#include "databento/with_ts_out.hpp"

static std::sig_atomic_t volatile gSignal;

int main() {
  std::unordered_map<std::uint32_t, std::string> symbol_mappings;

  auto client = databento::LiveBuilder{}
                    .SetKeyFromEnv()
                    .SetDataset(databento::dataset::kGlbxMdp3)
                    .BuildThreaded();
  std::cout << "Authenticated successfully\n";

  // Set up signal handler for Ctrl+C
  std::signal(SIGINT, [](int signal) { gSignal = signal; });

  std::vector<std::string> symbols{"ESM3", "ESM3 C4200", "ESM3 P4100"};
  client.Subscribe(symbols, databento::Schema::Definition,
                   databento::SType::RawSymbol);
  client.Subscribe(symbols, databento::Schema::Mbo,
                   databento::SType::RawSymbol);

  auto metadata_callback = [](databento::Metadata&& metadata) {
    std::cout << metadata << '\n';
  };
  auto record_callback = [&symbol_mappings](const databento::Record& rec) {
    using databento::RType;
    switch (rec.Header().rtype) {
      case RType::Mbo: {
        auto ohlcv = rec.Get<databento::WithTsOut<databento::MboMsg>>();
        std::cout << "Received tick for "
                  << symbol_mappings.at(ohlcv.rec.hd.instrument_id)
                  << " with ts_out " << ohlcv.ts_out.time_since_epoch().count()
                  << ": " << ohlcv.rec << '\n';
        break;
      }
      case RType::InstrumentDef: {
        std::cout << "Received definition: "
                  << rec.Get<databento::InstrumentDefMsg>() << '\n';
        break;
      }
      case RType::SymbolMapping: {
        auto mapping = rec.Get<databento::SymbolMappingMsg>();
        std::cout << "Received symbol mapping: " << mapping << '\n';
        symbol_mappings.emplace(mapping.hd.instrument_id,
                                mapping.stype_in_symbol.data());
        break;
      }
      case RType::System: {
        const auto& system_msg = rec.Get<databento::SystemMsg>();
        if (!system_msg.IsHeartbeat()) {
          std::cout << "Received system msg: " << system_msg.Msg() << '\n';
        }
        break;
      }
      case RType::Error: {
        std::cerr << "Received error from gateway: "
                  << rec.Get<databento::ErrorMsg>().Err() << '\n';
        break;
      }
      default: {
        std::cerr << "Received unknown record with rtype " << std::hex
                  << static_cast<std::uint16_t>(rec.Header().rtype) << '\n';
      }
    }
  };
  client.Start(metadata_callback, record_callback);
  while (::gSignal == 0) {
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
  }
  return 0;
}
