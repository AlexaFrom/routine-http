#include "http/response.hpp"
#include "http/body_storage.hpp"
#include "utils/utils.hpp"
#include <memory>
#include <spdlog/spdlog.h>
#include <sstream>

routine::http::Response::Response(Status status, Headers headers)
    : status_(status), headers_(headers), body_(nullptr) {}

routine::http::Response::Response(Status status, Headers headers,
                                  std::shared_ptr<I_BodyStorage> body)
    : status_(status), headers_(headers), body_(body) {}

routine::http::Response::Response(Status status, Headers headers, std::string body)
    : status_(status), headers_(headers), body_(std::make_shared<MemoryBody>()) {
  std::vector<uint8_t> buffer;
  buffer.resize(body.size());
  ::memcpy(buffer.data(), body.data(), buffer.size());
  body_->write(buffer);
}

routine::http::Response::Response(Status status, Headers headers, const std::vector<uint8_t>& body)
    : status_(status), headers_(headers), body_(std::make_shared<MemoryBody>()) {
  body_->write(body);
}

std::shared_ptr<routine::http::I_BodyStorage> routine::http::Response::body() {
  return body_;
}

routine::http::Status& routine::http::Response::status() {
  return status_;
}

routine::http::Headers& routine::http::Response::headers() {
  return headers_;
}

std::string routine::http::Response::prepare_response() {
  {
    if (!headers_.contains("server")) headers_.insert("server", "RoutineHttpLibrary");
    headers_.insert("date", routine::http::utils::get_current_http_date());

    if (body_) {
      // TODO # content-type = body_.get_type();
      if (!headers_.contains("content-type")) headers_.insert("content-type", "text/plain");
    }
    headers_["content-length"] = body_ ? std::to_string(body_->size()) : "0";
  }

  {
    std::ostringstream stream;
    stream << "HTTP/1.1 " << utils::to_string(status_) << "\r\n";
    for (auto& header : headers_)
      stream << header.as_string() << "\r\n";

    stream << "\r\n";
    if (body_) stream << body_->as_string();
    return stream.str();
  }
}
