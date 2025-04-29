#pragma once

#include "http/body_storage.hpp"
#include "http/headers.hpp"
#include "http/params.hpp"
#include "http/types.hpp"
#include <memory>

namespace routine::http {

  class Request {
  public:
    Request(const std::string& raw_http);

    const Headers& headers();
    Method method();
    const std::string& path();
    Version version();

    routine::http::Parameters& query_params() { return query_params_; }
    routine::http::Parameters& path_params() { return path_params_; }
    std::unique_ptr<routine::http::I_BodyStorage>& body() { return body_; };

  private:
    size_t parse_query_parameters(const std::string& string);

  private:
    Method method_;
    std::string path_;
    routine::http::Version version_;
    routine::http::Headers headers_;

    routine::http::Parameters query_params_;
    routine::http::Parameters path_params_;

    std::unique_ptr<I_BodyStorage> body_;
  };

  using Request_ptr = std::shared_ptr<Request>;

} // namespace routine::http
