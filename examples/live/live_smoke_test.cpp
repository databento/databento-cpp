#include <cassert>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <databento/constants.hpp>
#include <databento/datetime.hpp>
#include <databento/enums.hpp>
#include <databento/live.hpp>
#include <databento/symbology.hpp>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace db = databento;

std::vector<std::string> SplitSymbols(const std::string& symbols) {
  std::vector<std::string> result;
  std::stringstream ss(symbols);

  std::string buffer;
  while (std::getline(ss, buffer, ',')) {
    result.push_back(buffer);
  }

  return result;
}

std::pair<bool, db::UnixNanos> TryConvertToUnixNanos(const char* start) {
  std::size_t pos;
  const uint64_t result = std::stoul(start, &pos, 10);
  if (pos != std::strlen(start)) {
    return std::make_pair(false, db::UnixNanos{});
  }

  return std::make_pair(true, db::UnixNanos{std::chrono::nanoseconds(result)});
}

void ProcessRecords(db::LiveBlocking& client, db::Schema schema,
                    bool start_from_epoch) {
  client.Start();

  std::cout << "Starting client...\n";

  // For start != 0 we stop at SymbolMappingMsg so that the tests can be run
  // outside trading hours
  auto expected_rtype = db::Record::RTypeFromSchema(schema);
  if (!start_from_epoch) {
    expected_rtype = db::RType::SymbolMapping;
  }

  constexpr auto timeout = std::chrono::seconds{30};

  while (auto record = client.NextRecord(timeout)) {
    if (record->RType() == expected_rtype) {
      std::cout << "Received expected record type \n";
      break;
    } else if (auto* msg = record->GetIf<db::ErrorMsg>()) {
      std::stringstream ss;
      ss << "Received error " << msg->Err() << '\n';
      std::cerr << ss.str();
      throw std::runtime_error(ss.str());
    }
  }

  std::cout << "Finished client\n";
}

void ProcessSnapshotRecords(db::LiveBlocking& client) {
  client.Start();

  std::cout << "Starting client...\n";

  constexpr auto timeout = std::chrono::seconds{30};

  auto received_snapshot_record = false;

  while (auto record = client.NextRecord(timeout)) {
    if (auto* mbo_msg = record->GetIf<db::MboMsg>()) {
      if (mbo_msg->flags.IsSnapshot()) {
        received_snapshot_record = true;
      } else {
        std::cout << "Received expected record type\n";
        break;
      }
    } else if (auto* error_msg = record->GetIf<db::ErrorMsg>()) {
      std::stringstream ss;
      ss << "Received error " << error_msg->Err() << '\n';
      throw std::runtime_error(ss.str());
    }
  }

  std::cout << "Finished client\n";

  if (!received_snapshot_record) {
    throw std::runtime_error("Did not receive snapshot record");
  }
}

class ArgParser {
 public:
  struct Arg {
    const std::string name;
    const std::string arg;
    const char* value = nullptr;
  };

  void Add(Arg arg) { args.emplace_back(arg); }

  void Parse(int argc, char* argv[]) {
    for (auto i = 1; i < argc;) {
      const auto cur_arg = argv[i];
      auto it = std::find_if(args.begin(), args.end(), [&cur_arg](const auto& arg) {
        return cur_arg == arg.arg;
      });
      if (it != args.end()) {
        it->value = argv[i + 1];
      }
      i += 2;
    }
  }

  const char* Get(const std::string& arg_name) const {
    auto it = std::find_if(args.begin(), args.end(), [&arg_name](const auto& arg) {
      return arg_name == arg.name;
    });

    if (it == args.end()) {
      return nullptr;
    }

    return it->value;
  }

 private:
  std::vector<Arg> args;
};

ArgParser ParseArgs(int argc, char* argv[]) {
  ArgParser parser;
  parser.Add(ArgParser::Arg{"gateway", "--gateway"});
  parser.Add(ArgParser::Arg{"port", "--port", "13000"});
  parser.Add(
      ArgParser::Arg{"api_key_env_var", "--api-key-env-var", "DATABENTO_API_KEY"});
  parser.Add(ArgParser::Arg{"dataset", "--dataset"});
  parser.Add(ArgParser::Arg{"schema", "--schema"});
  parser.Add(ArgParser::Arg{"stype", "--stype"});
  parser.Add(ArgParser::Arg{"symbols", "--symbols"});
  parser.Add(ArgParser::Arg{"start", "--start"});
  parser.Add(ArgParser::Arg{"use_snapshot", "--use-snapshot", "0"});

  parser.Parse(argc, argv);

  return parser;
}

int main(int argc, char* argv[]) {
  const auto parser = ParseArgs(argc, argv);

  const auto gateway = parser.Get("gateway");
  const auto port = std::atoi(parser.Get("port"));
  const auto api_key_env_var = parser.Get("api_key_env_var");
  const auto dataset = db::FromString<db::Dataset>(parser.Get("dataset"));
  const auto schema = db::FromString<db::Schema>(parser.Get("schema"));
  const auto stype = db::FromString<db::SType>(parser.Get("stype"));
  const auto symbols = SplitSymbols(parser.Get("symbols"));
  const auto start = parser.Get("start");
  const auto use_snapshot = std::atoi(parser.Get("use_snapshot"));

  const auto api_key = std::getenv(api_key_env_var);
  assert(api_key);

  auto client = db::LiveBlocking::Builder()
                    .SetAddress(gateway, static_cast<uint16_t>(port))
                    .SetKey(std::string{api_key})
                    .SetDataset(dataset)
                    .BuildBlocking();

  bool start_from_epoch = false;

  if (use_snapshot) {
    client.SubscribeWithSnapshot(symbols, schema, stype);
  } else if (start) {
    const auto [success, start_nanos] = TryConvertToUnixNanos(start);
    if (success) {
      start_from_epoch = start_nanos.time_since_epoch().count() == 0;
      client.Subscribe(symbols, schema, stype, start_nanos);
    } else {
      client.Subscribe(symbols, schema, stype, start);
    }
  } else {
    client.Subscribe(symbols, schema, stype);
  }

  if (use_snapshot) {
    ProcessSnapshotRecords(client);
  } else {
    ProcessRecords(client, schema, start_from_epoch);
  }

  std::cout << "Finished client\n";

  return 0;
}
