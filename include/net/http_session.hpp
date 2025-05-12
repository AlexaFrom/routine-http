#pragma once

#include "http/request.hpp"
#include "http/response.hpp"
#include "scheduler.hpp"
#include <chrono>
#include <functional>
#include <memory>
#include <spdlog/logger.h>
#include <system_error>

#ifdef USE_BOOST_ASIO
#include <boost/asio.hpp>
using namespace boost;
#else
#include <asio.hpp>
#endif

namespace routine::net {

  class HttpSession : spdlog::logger, public std::enable_shared_from_this<HttpSession> {
  public:
    HttpSession(routine::Scheduler_ptr scheduler, asio::ip::tcp::socket socket);
    HttpSession(routine::Scheduler_ptr scheduler, const std::string& endpoint);

    void run_process();

    void set_timeout(std::chrono::milliseconds timeout);
    std::chrono::milliseconds get_timeout() const;

    void send_response(routine::http::Response_ptr response,
                       std::function<void(const std::error_code&)> callback = nullptr);

    void send_request(
        routine::http::Request_ptr request,
        std::function<void(const std::error_code&, http::Response_ptr)> callback = nullptr);

    void read_response(
        std::function<void(const std::error_code&, routine::http::Response_ptr)> callback);
    void
    read_request(std::function<void(const std::error_code&, routine::http::Request_ptr)> callback);

    void close(const std::error_code& ec);

  private:
    void run_timeout_timer();

    bool is_errors(const std::error_code& ec);

  private:
    std::string address_;

    routine::Scheduler_ptr scheduler_;
    asio::ip::tcp::socket socket_;

    asio::steady_timer timeout_timer_;
    std::chrono::milliseconds timeout_;

  private:
    using Buffer_ptr = std::shared_ptr<asio::streambuf>;

    template <typename T>
    void do_read_headers(std::function<void(const std::error_code&, std::shared_ptr<T>)> callback);

    void do_prepare_and_read_body(
        routine::http::Request_ptr request, Buffer_ptr buffer,
        std::function<void(const std::error_code&, routine::http::Request_ptr)> callback);
    void do_prepare_and_read_body(
        routine::http::Response_ptr response, Buffer_ptr buffer,
        std::function<void(const std::error_code&, routine::http::Response_ptr)> callback);

    template <typename T>
    void do_read_body(std::shared_ptr<T> message, Buffer_ptr buffer,
                      std::function<void(const std::error_code&, std::shared_ptr<T>)> callback);
  };

  using HttpSession_ptr = std::shared_ptr<HttpSession>;

} // namespace routine::net
