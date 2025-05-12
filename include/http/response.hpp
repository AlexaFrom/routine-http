#pragma once

#include "body_storage.hpp"
#include "headers.hpp"
#include "http/body_storage.hpp"
#include "http/headers.hpp"
#include "http/types.hpp"
#include "types.hpp"
#include "utils/utils.hpp"
#include <memory>
#include <numeric>
#include <stdexcept>
#include <system_error>
#include <vector>

namespace routine::http {

  class Response;

  using Response_ptr = std::shared_ptr<routine::http::Response>;

  class Response {
  public:
    Response(std::string raw_http) { throw std::runtime_error("Not impl"); }
    Response(routine::http::Status status, Headers headers);
    Response(Status status, Headers headers, std::shared_ptr<I_BodyStorage> body);
    Response(Status status, Headers headers, std::string body);
    Response(Status status, Headers headers, const std::vector<uint8_t>& body);

    std::shared_ptr<I_BodyStorage> body();
    Status& status();
    Headers& headers();

    std::string prepare_response();

  private:
    Status status_;
    Headers headers_;
    std::shared_ptr<I_BodyStorage> body_;
  };

} // namespace routine::http
