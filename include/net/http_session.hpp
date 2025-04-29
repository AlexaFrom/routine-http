#pragma once

#include "http/request.hpp"
#include "request_handler.hpp"
#include "scheduler.hpp"
#include "utils/loggable_object.hpp"
#include <memory>
#include <system_error>

namespace routine::net {

  class HttpSession : private utils::LoggableObject,
                      public std::enable_shared_from_this<HttpSession> {
  public:
    HttpSession(std::shared_ptr<routine::Scheduler> scheduler, asio::ip::tcp::socket socket);

    void run();

  private:
    void do_read_headers();
    void on_read_headers(const std::error_code& ec, size_t bytes);

    void do_read_body();
    void do_read_chunked_body();
    void on_read_body(const std::error_code& ec, size_t);

    void on_request_ready();
    void send_response();

    bool is_errors(const std::error_code& ec);
    void close(std::error_code ec);

  private:
    asio::streambuf buffer_;
    routine::Scheduler_ptr scheduler_;
    asio::ip::tcp::socket socket_;

    http::Request_ptr request_;
    http::Response_ptr response_;
    std::shared_ptr<http::RequestHandler> handler_;
  };

} // namespace routine::net
