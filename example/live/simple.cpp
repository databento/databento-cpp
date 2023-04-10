#include <csignal>  // sig_atomic_t
#include <iostream>

#include "databento/constants.hpp"
#include "databento/dbn.hpp"
#include "databento/enums.hpp"
#include "databento/live.hpp"
#include "databento/live_threaded.hpp"
#include "databento/record.hpp"

static std::sig_atomic_t volatile gSignal;

int main() {
  auto client = databento::LiveBuilder{}
                    .SetKeyFromEnv()
                    .SetDataset(databento::dataset::kGlbxMdp3)
                    .BuildThreaded();
  std::cout << "Authenticated successfully\n";

  // Set up signal handler for Ctrl+C
  std::signal(SIGINT, [](int signal) { gSignal = signal; });

  client.Subscribe({"ESZ4"}, databento::Schema::Definition,
                   databento::SType::Native);

  auto metadata_callback = [](databento::Metadata&& metadata) {
    std::cout << metadata << '\n';
  };
  auto record_callback = [](const databento::Record& rec) {
    using databento::RType;
    switch (rec.Header().rtype) {
      case RType::InstrumentDef: {
        std::cout << "Received definition: "
                  << rec.Get<databento::InstrumentDefMsg>() << '\n';
        break;
      }
      case RType::SymbolMapping: {
        std::cout << "Received symbol mapping: "
                  << rec.Get<databento::SymbolMappingMsg>() << '\n';
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
                  << rec.Header().rtype << '\n';
      }
    }
  };
  client.Start(metadata_callback, record_callback);
  while (::gSignal == 0) {
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
  }
  return 0;
}
