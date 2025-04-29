#pragma once

#include "net/http_session.hpp"
#include "scheduler.hpp"
#include "utils/loggable_object.hpp"
#include <chrono>
#include <functional>
#include <memory>
#include <system_error>
#include <thread>

#ifdef USE_BOOST_ASIO
#include <boost/asio.hpp>
using namespace boost;
#else
#include <asio.hpp>
#endif

namespace routine::net {

  template <typename Session>
  class Acceptor : private utils::LoggableObject {
  public:
    Acceptor(routine::Scheduler_ptr scheduler, short port)
        : scheduler_(scheduler),
          acceptor_(scheduler->get_context(), asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
          LoggableObject("Acceptor") {}

    void async_accept() {
      using namespace std::placeholders;
      acceptor_.async_accept(std::bind(&Acceptor::on_accept, this, _1, _2));
    }

    void on_accept(const std::error_code& ec, asio::ip::tcp::socket socket) {
      async_accept();

      if (ec) {
        warn("Some errors while accepting connection. {} - {}", ec.value(), ec.message());
        return;
      }

      trace("New connection: {}:{}", socket.remote_endpoint().address().to_v4().to_string(),
            socket.remote_endpoint().port());

      std::make_shared<routine::net::HttpSession>(scheduler_, std::move(socket))->run();
    }

  private:
    asio::ip::tcp::acceptor acceptor_;
    routine::Scheduler_ptr scheduler_;
  };

} // namespace routine::net
