#include "net/http_session.hpp"
#include "http/body_storage.hpp"
#include "http/headers.hpp"
#include "http/request.hpp"
#include "http/response.hpp"
#include "http/types.hpp"
#include <chrono>
#include <cstring>
#include <functional>
#include <memory>
#include <spdlog/spdlog.h>
#include <system_error>

routine::net::HttpSession::HttpSession(routine::Scheduler_ptr scheduler,
                                       asio::ip::tcp::socket socket)
    : spdlog::logger(*spdlog::get("Http")), scheduler_(std::move(scheduler)),
      socket_(std::move(socket)), timeout_timer_(scheduler_->get_context()),
      timeout_(std::chrono::milliseconds(1000)) {
  address_ = socket_.is_open()
                 ? std::format("{}:{}", socket_.remote_endpoint().address().to_string(),
                               socket_.remote_endpoint().port())
                 : "Null";
}

routine::net::HttpSession::HttpSession(routine::Scheduler_ptr scheduler,
                                       const std::string& endpoint)
    : spdlog::logger(*spdlog::get("Http")), scheduler_(std::move(scheduler)),
      socket_(scheduler_->get_context()), timeout_timer_(scheduler_->get_context()),
      timeout_(std::chrono::milliseconds(1000)) {
  asio::ip::tcp::resolver resolver(scheduler_->get_context());

  trace("Connecting to {}", endpoint);
  auto endpoints = resolver.resolve(endpoint, "80");
  asio::connect(socket_, endpoints);

  address_ = socket_.is_open()
                 ? std::format("{}:{}", socket_.remote_endpoint().address().to_string(),
                               socket_.remote_endpoint().port())
                 : "Null";
}

void routine::net::HttpSession::run_process() {
  timeout_timer_.cancel();
  read_request(
      [self = shared_from_this()](const std::error_code& ec, routine::http::Request_ptr request) {
        if (self->is_errors(ec)) return;

        self->scheduler_->prepare_task([self = std::move(self), req = std::move(request)]() {
          auto handler = self->scheduler_->route_request(req);
          if (handler) {
            auto response = handler->process_request(req);
            if (response) {
              self->send_response(response, [self](const std::error_code& ec) {});
            } else {
              self->send_response(std::make_shared<http::Response>(
                  http::Status::Internal_Server_Error, http::Headers{},
                  fmt::format("Resource handler '{}' did not return a response", req->path())));
            }
          } else {
            self->send_response(std::make_shared<http::Response>(
                http::Status::Not_Found, http::Headers{},
                fmt::format("Requested resource '{}' handler not found", req->path())));
          }
          if (req->headers().contains(http::Header::Connection) &&
              (req->headers().at(http::Header::Connection) == "close" ||
               req->headers().at(http::Header::Connection) == "Close"))
            self->close({});
          else
            self->run_process();
        });
      });
}

void routine::net::HttpSession::set_timeout(std::chrono::milliseconds timeout) {
  timeout_ = timeout;
}

std::chrono::milliseconds routine::net::HttpSession::get_timeout() const {
  return timeout_;
}

void routine::net::HttpSession::close(const std::error_code& ec) {
  if (!socket_.is_open()) return;

  socket_.cancel();

#ifdef USE_BOOST_ASIO
  boost::system::error_code error_code;
#else
  std::error_code error_code;
#endif

  // closing errors ignored
  socket_.shutdown(asio::socket_base::shutdown_both, error_code);
  socket_.close(error_code);

  debug("Session {} was closed by #{} - {}", address_, ec.value(), ec.message());
}

void routine::net::HttpSession::run_timeout_timer() {
  timeout_timer_.expires_after(timeout_);
  timeout_timer_.async_wait([self = shared_from_this()](const std::error_code& ec) {
    if (!ec) self->close(std::make_error_code(std::errc::timed_out));
  });
}

bool routine::net::HttpSession::is_errors(const std::error_code& ec) {
  timeout_timer_.cancel();
  static const std::unordered_set<size_t> ignoring_error_codes{125};
  if (ec && !ignoring_error_codes.contains(ec.value())) {
    error("Session {}. Error code #{} - {}", address_, ec.value(), ec.message());
    close(ec);
    return true;
  }
  return !socket_.is_open();
}

void routine::net::HttpSession::send_response(
    routine::http::Response_ptr response, std::function<void(const std::error_code&)> callback) {
  if (!socket_.is_open()) {
    if (callback) callback(std::make_error_code(std::errc::not_connected));
    return;
  }
  if (!response) {
    if (callback) callback(std::make_error_code(std::errc::invalid_argument));
    return;
  }

  std::string buffer = response->prepare_response();
  socket_.async_write_some(asio::buffer(buffer),
                           [self = shared_from_this(), cb = std::move(callback),
                            response](const std::error_code& ec, size_t) {
                             if (cb) cb(ec);
                           });
  run_timeout_timer();
}

void routine::net::HttpSession::send_request(
    routine::http::Request_ptr request,
    std::function<void(const std::error_code&, http::Response_ptr)> callback) {
  if (!socket_.is_open()) {
    if (callback) callback(std::make_error_code(std::errc::not_connected), nullptr);
    return;
  }
  if (!request) {
    if (callback) callback(std::make_error_code(std::errc::invalid_argument), nullptr);
    return;
  }

  std::string buffer = request->prepare_request();
  socket_.async_write_some(asio::buffer(buffer),
                           [self = shared_from_this(), cb = std::move(callback),
                            request](const std::error_code& ec, size_t) {
                             if (cb) self->read_response(std::move(cb));
                           });
}

void routine::net::HttpSession::read_response(
    std::function<void(const std::error_code&, routine::http::Response_ptr)> callback) {
  if (!callback) return;

  do_read_headers<http::Response>(std::move(callback));
}

void routine::net::HttpSession::read_request(
    std::function<void(const std::error_code&, routine::http::Request_ptr)> callback) {
  if (!callback) return;

  do_read_headers<http::Request>(std::move(callback));
}

template <typename T>
void routine::net::HttpSession::do_read_headers(
    std::function<void(const std::error_code&, std::shared_ptr<T>)> callback) {
  if (!socket_.is_open()) {
    callback(std::make_error_code(std::errc::not_connected), nullptr);
    return;
  }

  auto buffer = std::make_shared<asio::streambuf>();
  asio::async_read_until(
      socket_, *buffer, "\r\n\r\n",
      [self = shared_from_this(), buffer, cb = std::move(callback)](const std::error_code& ec,
                                                                    size_t bytes) {
        if (self->is_errors(ec)) {
          cb(ec, nullptr);
          return;
        }
        std::string str;
        str.resize_and_overwrite(bytes - 2, [&buffer](char* data, size_t size) {
          std::memcpy(data, buffer->data().data(), size);
          return size;
        });
        buffer->consume(bytes);
        auto object = std::make_shared<T>(str);

        bool is_content_length_have = object->headers().contains(http::Header::Content_Length);
        bool is_chunked_body = !is_content_length_have &&
                               (object->headers().contains(http::Header::Transfer_Encoding) &&
                                object->headers()[http::Header::Transfer_Encoding] == "chunked");

        if (is_chunked_body) {
          self->warn("Session {}. 'Transfer-Encoding: Chunked' not yet support", self->address_);
          cb(std::make_error_code(std::errc::not_supported), object);
          return;
        }

        if (is_content_length_have) {
          self->do_prepare_and_read_body(std::move(object), buffer, std::move(cb));
        } else {
          cb(std::error_code{}, object);
        }
      });
  run_timeout_timer();
}
template void routine::net::HttpSession::do_read_headers<routine::http::Request>(
    std::function<void(const std::error_code&, std::shared_ptr<routine::http::Request>)>);
template void routine::net::HttpSession::do_read_headers<routine::http::Response>(
    std::function<void(const std::error_code&, std::shared_ptr<routine::http::Response>)>);

void routine::net::HttpSession::do_prepare_and_read_body(
    routine::http::Request_ptr request, Buffer_ptr buffer,
    std::function<void(const std::error_code&, routine::http::Request_ptr)> callback) {
  auto handler = scheduler_->route_request(request);
  if (handler) {
    handler->prepare_request(request);
  } else {
    if (request->headers().contains(http::Header::Content_Type) &&
        request->headers().at(http::Header::Content_Type) == "application/json")
      request->body() = std::make_unique<http::JsonBody>();
    else
      request->body() = std::make_unique<http::MemoryBody>();
  }

  do_read_body(request, buffer, std::move(callback));
}

void routine::net::HttpSession::do_prepare_and_read_body(
    routine::http::Response_ptr response, Buffer_ptr buffer,
    std::function<void(const std::error_code&, routine::http::Response_ptr)> callback) {
  if (response->headers().contains(http::Header::Content_Type) &&
      response->headers().at(http::Header::Content_Type) == "application/json")
    response->body() = std::make_unique<http::JsonBody>();
  else
    response->body() = std::make_unique<http::MemoryBody>();

  do_read_body(response, buffer, std::move(callback));
}

template <typename T>
void routine::net::HttpSession::do_read_body(
    std::shared_ptr<T> message, Buffer_ptr buffer,
    std::function<void(const std::error_code&, std::shared_ptr<T>)> callback) {
  size_t content_length = std::stoull(message->headers().at(http::Header::Content_Length));
  size_t body_not_loaded = content_length - buffer->size();

  if (body_not_loaded > 0) {
#ifdef USE_BOOST_ASIO
    boost::system::error_code error_code;
#else
    std::error_code error_code;
#endif
    run_timeout_timer();
    asio::read(socket_, *buffer, asio::transfer_exactly(body_not_loaded), error_code);
    if (is_errors(error_code)) {
      callback(error_code, std::move(message));
      return;
    }
  }

  message->body()->write(*buffer);
  callback(std::error_code(), message);
}

template void routine::net::HttpSession::do_read_body<routine::http::Request>(
    std::shared_ptr<http::Request>, Buffer_ptr,
    std::function<void(const std::error_code&, std::shared_ptr<http::Request>)>);

template void routine::net::HttpSession::do_read_body<routine::http::Response>(
    std::shared_ptr<http::Response>, Buffer_ptr,
    std::function<void(const std::error_code&, std::shared_ptr<http::Response>)>);
