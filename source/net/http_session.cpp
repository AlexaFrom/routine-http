#include "net/http_session.hpp"
#include "http/headers.hpp"
#include "http/response.hpp"
#include "http/types.hpp"
#include <boost/asio/post.hpp>

routine::net::HttpSession::HttpSession(routine::Scheduler_ptr scheduler,
                                       asio::ip::tcp::socket socket)
    : LoggableObject("Http"), scheduler_(scheduler), socket_(std::move(socket)) {}

void routine::net::HttpSession::run() {
  do_read_headers();
}

void routine::net::HttpSession::do_read_headers() {
  using namespace std::placeholders;
  buffer_.consume(buffer_.size());
  asio::async_read_until(socket_, buffer_, "\r\n\r\n",
                         std::bind(&HttpSession::on_read_headers, shared_from_this(), _1, _2));
}

void routine::net::HttpSession::on_read_headers(const std::error_code& ec, size_t bytes) {
  if (is_errors(ec)) return;

  request_ = std::make_unique<http::Request>(
      std::string{asio::buffers_begin(buffer_.data()), asio::buffers_end(buffer_.data())});
  buffer_.consume(bytes);

  handler_ = scheduler_->route_request(request_);
  if (!handler_) {
    std::string msg = std::format("Requested resource '{}' handler not found", request_->path());
    warn(msg);

    response_ = std::make_unique<http::Response>(http::Status::BAD_REQUEST, http::Headers{}, msg);
    send_response();
    return;
  }

  response_ = handler_->prepare_request(request_);
  if (response_ != nullptr) {
    // Ответ был сформирован на этапе подготовки запроса, тело не нужно,
    // отправляем response
    send_response();
    return;
  }

  if (request_->body()) {
    // если обработчик подразумевает наличие тела, т.е. он же его и создал
    if (request_->headers().contains("content-length")) {
      do_read_body();
      return;
    }
    const bool isChunkedBody = request_->headers().contains("transfer-encoding") &&
                               request_->headers()["transfer-encoding"] == "chunked";
    if (isChunkedBody) {
      do_read_chunked_body();
    } else {
      warn("Handler for resource '{}' expected to get the body, but the request does not contain "
           "any information about the body",
           request_->path());
      on_request_ready();
    }
  } else {
    on_request_ready();
  }
}

void routine::net::HttpSession::do_read_body() {
  using namespace std::placeholders;
  size_t content_length = std::stoull(request_->headers()["content-length"]);

  if (buffer_.size() <= content_length) {
    debug("Body is already contained in buffer");
    on_read_body({}, buffer_.size());
  } else
    asio::async_read(socket_, buffer_, asio::transfer_exactly(content_length - buffer_.size()),
                     std::bind(&HttpSession::on_read_body, shared_from_this(), _1, _2));
}

void routine::net::HttpSession::do_read_chunked_body() {
  error("Chunked body not supported so far");
  response_ = std::make_unique<http::Response>(http::Status::BAD_REQUEST, http::Headers{},
                                               "Chunked body not supported so far");
  send_response();
}

void routine::net::HttpSession::on_read_body(const std::error_code& ec, size_t) {
  if (is_errors(ec)) return;

  request_->body()->write(buffer_);
  on_request_ready();
}

void routine::net::HttpSession::on_request_ready() {
  // clear buffer
  buffer_.consume(buffer_.size());
  scheduler_->prepare_task([self = shared_from_this()]() {
    self->response_ = self->handler_->process_request(self->request_);
    self->send_response();
  });
}

void routine::net::HttpSession::send_response() {
  if (response_) {
    std::string response_buffer = response_->prepare_response();
    socket_.async_write_some(asio::buffer(response_buffer),
                             [self = shared_from_this()](const std::error_code& ec, size_t bytes) {
                               if (self->is_errors(ec)) return;

                               self->debug("Response sended OK");
                               bool isKeepAlive =
                                   self->request_->headers().contains("connection") &&
                                   (self->request_->headers()["connection"] == "Keep-Alive" ||
                                    self->request_->headers()["connection"] == "keep-alive");
                               self->response_ = nullptr;
                               self->request_ = nullptr;

                               if (isKeepAlive) {
                                 self->do_read_headers();
                               } else {
                                 self->close({});
                               }
                             });
  } else {
    warn("No response available in Session {}:{}", socket_.remote_endpoint().address().to_string(),
         socket_.remote_endpoint().port());
    response_ = std::make_unique<http::Response>(
        http::Status::INTERNAL_SERVER_ERROR, http::Headers{}, "No response received from task...");
    send_response();
  }
}

bool routine::net::HttpSession::is_errors(const std::error_code& ec) {
  if (ec) {
    error("#{} - {}", ec.value(), ec.message());
    close(ec);
    return true;
  }
  return false;
}

void routine::net::HttpSession::close(std::error_code ec) {
  debug("Closing socket {}:{} by {} {}. Session aborted.",
        socket_.remote_endpoint().address().to_v4().to_string(), socket_.remote_endpoint().port(),
        ec.value(), ec.message());
  socket_.shutdown(asio::socket_base::shutdown_both);
  socket_.close();
}
