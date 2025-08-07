#include <csignal>  // sig_atomic_t
#include <cstdint>
#include <databento/constants.hpp>
#include <databento/dbn.hpp>
#include <databento/enums.hpp>
#include <databento/live.hpp>
#include <databento/live_threaded.hpp>
#include <databento/log.hpp>
#include <databento/record.hpp>
#include <databento/symbol_map.hpp>
#include <databento/with_ts_out.hpp>
#include <iostream>
#include <memory>

namespace db = databento;

static std::sig_atomic_t volatile gSignal;

int main() {
  db::PitSymbolMap symbol_mappings;
  auto log_receiver = std::make_unique<db::ConsoleLogReceiver>(db::LogLevel::Debug);

  auto client = db::LiveThreaded::Builder()
                    .SetLogReceiver(log_receiver.get())
                    .SetSendTsOut(true)
                    .SetKeyFromEnv()
                    .SetDataset(db::Dataset::GlbxMdp3)
                    .BuildThreaded();

  // Set up signal handler for Ctrl+C
  std::signal(SIGINT, [](int signal) { gSignal = signal; });

  std::vector<std::string> symbols{"ESZ5", "ESZ5 C6200", "ESZ5 P5500"};
  client.Subscribe(symbols, db::Schema::Definition, db::SType::RawSymbol);
  client.Subscribe(symbols, db::Schema::Mbo, db::SType::RawSymbol);

  auto metadata_callback = [](db::Metadata&& metadata) {
    std::cout << metadata << '\n';
  };
  auto record_callback = [&symbol_mappings](const db::Record& rec) {
    using db::RType;
    switch (rec.RType()) {
      case RType::Mbo: {
        auto ohlcv = rec.Get<db::WithTsOut<db::MboMsg>>();
        std::cout << "Received tick for " << symbol_mappings[ohlcv.rec.hd.instrument_id]
                  << " with ts_out " << ohlcv.ts_out.time_since_epoch().count() << ": "
                  << ohlcv.rec << '\n';
        break;
      }
      case RType::InstrumentDef: {
        std::cout << "Received definition: " << rec.Get<db::InstrumentDefMsg>() << '\n';
        break;
      }
      case RType::SymbolMapping: {
        auto mapping = rec.Get<db::SymbolMappingMsg>();
        symbol_mappings.OnSymbolMapping(mapping);
        break;
      }
      case RType::System: {
        const auto& system_msg = rec.Get<db::SystemMsg>();
        if (!system_msg.IsHeartbeat()) {
          std::cout << "Received system msg: " << system_msg.Msg() << '\n';
        }
        break;
      }
      case RType::Error: {
        std::cerr << "Received error from gateway: " << rec.Get<db::ErrorMsg>().Err()
                  << '\n';
        break;
      }
      default: {
        std::cerr << "Received unknown record with rtype " << std::hex
                  << static_cast<std::uint16_t>(rec.RType()) << '\n';
      }
    }
    return db::KeepGoing::Continue;
  };
  client.Start(metadata_callback, record_callback);
  while (::gSignal == 0) {
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
  }
  return 0;
}
