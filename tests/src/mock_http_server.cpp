#include "mock/mock_http_server.hpp"

#include <gtest/gtest.h>  // EXPECT_*
#include <httplib.h>

#include <cstddef>
#include <fstream>   // ifstream
#include <ios>       // streamsize
#include <iostream>  // cerr
#include <vector>

#include "databento/constants.hpp"
#include "databento/dbn.hpp"
#include "databento/dbn_encoder.hpp"
#include "databento/detail/zstd_stream.hpp"
#include "databento/record.hpp"

using databento::tests::mock::MockHttpServer;

int MockHttpServer::ListenOnThread() {
  listen_thread_ =
      detail::ScopedThread{[this] { this->server_.listen_after_bind(); }};
  return port_;
}

void MockHttpServer::MockBadRequest(const std::string& path,
                                    const nlohmann::json& json) {
  server_.Get(path, [json](const httplib::Request&, httplib::Response& resp) {
    resp.status = 400;
    resp.body = json.dump();
  });
}

void MockHttpServer::MockGetJson(const std::string& path,
                                 const nlohmann::json& json) {
  this->MockGetJson(path, {}, json);
}

void MockHttpServer::MockGetJson(
    const std::string& path, const std::map<std::string, std::string>& params,
    const nlohmann::json& json) {
  this->MockGetJson(path, params, json, {});
}

void MockHttpServer::MockGetJson(
    const std::string& path, const std::map<std::string, std::string>& params,
    const nlohmann::json& json, const nlohmann::json& warnings) {
  server_.Get(path, [json, params, warnings](const httplib::Request& req,
                                             httplib::Response& resp) {
    if (!req.has_header("Authorization")) {
      resp.status = 401;
      return;
    }
    [[maybe_unused]] auto _auth = req.get_header_value("Authorization");
    CheckParams(params, req);
    if (!warnings.empty()) {
      resp.set_header("X-Warning", warnings.dump());
    }
    resp.set_content(json.dump(), "application/json");
    resp.status = 200;
  });
}

void MockHttpServer::MockPostJson(
    const std::string& path,
    const std::map<std::string, std::string>& form_params,
    const nlohmann::json& json) {
  server_.Post(path, [json, form_params](const httplib::Request& req,
                                         httplib::Response& resp) {
    if (!req.has_header("Authorization")) {
      resp.status = 401;
      return;
    }
    [[maybe_unused]] auto _auth = req.get_header_value("Authorization");
    CheckFormParams(form_params, req);
    resp.set_content(json.dump(), "application/json");
    resp.status = 200;
  });
}

void MockHttpServer::MockStreamDbn(
    const std::string& path, const std::map<std::string, std::string>& params,
    const std::string& dbn_path) {
  constexpr std::size_t kChunkSize = 32;

  // Read contents into buffer
  std::ifstream input_file{dbn_path, std::ios::binary | std::ios::ate};
  const auto size = static_cast<std::size_t>(input_file.tellg());
  input_file.seekg(0, std::ios::beg);
  std::vector<char> buffer(size);
  input_file.read(buffer.data(), static_cast<std::streamsize>(size));

  // Serve
  server_.Get(path, [buffer = std::move(buffer), kChunkSize, params](
                        const httplib::Request& req, httplib::Response& resp) {
    if (!req.has_header("Authorization")) {
      resp.status = 401;
      return;
    }
    CheckParams(params, req);
    resp.status = 200;
    resp.set_header("Content-Disposition", "attachment; filename=test.dbn.zst");
    resp.set_content_provider(
        "application/octet-stream",
        [buffer, kChunkSize](const std::size_t offset,
                             httplib::DataSink& sink) {
          if (offset < buffer.size()) {
            sink.write(&buffer[offset],
                       std::min(kChunkSize, buffer.size() - offset));
          } else {
            sink.done();
          }
          return true;
        });
  });
}

void MockHttpServer::MockStreamDbn(
    const std::string& path, const std::map<std::string, std::string>& params,
    Record record, std::size_t count, std::size_t chunk_size) {
  MockStreamDbn(path, params, record, count, 0, chunk_size);
}

void MockHttpServer::MockStreamDbn(
    const std::string& path, const std::map<std::string, std::string>& params,
    Record record, std::size_t count, std::size_t extra_bytes,
    std::size_t chunk_size) {
  // Needs to be copy-constructable for use in server
  auto buffer = std::make_shared<detail::Buffer>();
  {
    detail::ZstdCompressStream zstd_stream{buffer.get()};
    DbnEncoder encoder{Metadata{
                           kDbnVersion,
                           ToString(Dataset::IfusImpact),
                           {Schema::Mbp1},
                       },
                       &zstd_stream};
    for (std::size_t i = 0; i < count; ++i) {
      encoder.EncodeRecord(record);
    }
    if (extra_bytes > sizeof(RecordHeader)) {
      std::vector<std::byte> empty(extra_bytes - sizeof(RecordHeader));
      // write the header so it looks like the start of a valid record
      zstd_stream.WriteAll(reinterpret_cast<const std::byte*>(&record.Header()),
                           sizeof(RecordHeader));
      zstd_stream.WriteAll(empty.data(), empty.size());
    }
  }
  server_.Get(path, [params, buffer, count, record, chunk_size](
                        const httplib::Request& req, httplib::Response& resp) {
    if (!req.has_header("Authorization")) {
      resp.status = 401;
      return;
    }
    CheckParams(params, req);
    resp.status = 200;
    resp.set_header("Content-Disposition", "attachment; filename=test.dbn.zst");
    resp.set_content_provider(
        "application/octet-stream",
        [&buffer, chunk_size](const std::size_t offset,
                              httplib::DataSink& sink) {
          if (buffer->ReadCapacity() - offset) {
            const auto write_size =
                std::min(chunk_size, buffer->ReadCapacity() - offset);
            sink.write(
                reinterpret_cast<const char*>(&buffer->ReadBegin()[offset]),
                write_size);
          } else {
            sink.done();
          }
          return true;
        });
  });
}

void MockHttpServer::CheckParams(
    const std::map<std::string, std::string>& params,
    const httplib::Request& req) {
  for (const auto& param : params) {
    EXPECT_TRUE(req.has_param(param.first))
        << "Missing query param " << param.first;
    EXPECT_EQ(req.get_param_value(param.first), param.second)
        << "Incorrect query param value for " << param.first << ". Expected "
        << param.second << ", found " << req.get_param_value(param.first);
  }
}

void MockHttpServer::CheckFormParams(
    const std::map<std::string, std::string>& params,
    const httplib::Request& req) {
  EXPECT_EQ(req.get_header_value("content-type"),
            "application/x-www-form-urlencoded")
      << "Request body is not form data";
  httplib::Params form_params;
  httplib::detail::parse_query_text(req.body, form_params);
  for (const auto& param : params) {
    const auto param_it = form_params.find(param.first);
    if (param_it == form_params.end()) {
      EXPECT_NE(param_it, form_params.end())
          << "Missing for mparam " << param.first;
    } else {
      EXPECT_EQ(param_it->second, param.second)
          << "Incorrect form param value for " << param.first << ". Expected "
          << param.second << ", found " << param_it->second;
    }
  }
}
