#include "net/acceptor.hpp"
#include "net/http_session.hpp"
#include "request_handler.hpp"
#include "scheduler.hpp"
#include "utils/benchmark.hpp"

#include <chrono>
#include <memory>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <thread>

class DynamicEchoHandler : public routine::http::RequestHandler {
public:
  // DONT FORGET TO SPECIFY THE PATH OF RESOURCE HANDLER
  inline static const std::string path{"/api/echo/{argumet}/hello"};

  // routine::http::Response_ptr prepare_request(routine::http::Request_ptr request) override {
  //   if (request->method() != routine::http::Method::Head)
  //     // request->body() = std::make_unique<routine::http::JsonBody>();
  //   return nullptr;
  // }

  routine::http::Response_ptr process_request(routine::http::Request_ptr request) override {
    return nullptr;
  }
};

class StaticEchoHandler : public routine::http::RequestHandler {
public:
  // DONT FORGET TO SPECIFY THE PATH OF RESOURCE HANDLER
  inline static const std::string path{"/api/echo"};

  routine::http::Response_ptr process_request(routine::http::Request_ptr request) override {
    auto response = std::make_unique<routine::http::Response>(
        routine::http::Status::OK, routine::http::Headers{}, std::move(request->body()));

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    return response;
  }
};

int main() {
  // Initialize loggers
  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  console_sink->set_level(spdlog::level::trace);
  std::vector<spdlog::sink_ptr> sinks{console_sink};

  auto logger = routine::utils::make_logger("System", spdlog::level::trace, sinks);
  routine::utils::make_logger("Benchmark", spdlog::level::trace, sinks);
  routine::utils::make_logger("Scheduler", spdlog::level::trace, sinks);
  routine::utils::make_logger("Acceptor", spdlog::level::trace, sinks);
  routine::utils::make_logger("Http", spdlog::level::trace, sinks);
  routine::utils::make_logger("Router", spdlog::level::trace, sinks);
  routine::utils::make_logger("ThreadPool", spdlog::level::trace, sinks);

  // Routing handlers
  auto router = std::make_unique<routine::http::RouteHandler>();

  router->add_handler<StaticEchoHandler>();
  router->add_handler<DynamicEchoHandler>();

  // Init main components

  auto scheduler = std::make_shared<routine::Scheduler>();

  auto acceptor =
      std::make_unique<routine::net::Acceptor<routine::net::HttpSession>>(scheduler, 8484);
  // routine::net::Acceptor<routine::net::HttpsSession> acceptor(scheduler, 443);

  scheduler->set_router(std::move(router));

  // Start

  acceptor->async_accept();

  scheduler->run(1, 2);
  scheduler->join_threads();
  // scheduler->run_service(service_acceptor);

  return 0;
}
