#pragma once

#include "http/request.hpp"
#include "http/route_handler.hpp"
#include "request_handler.hpp"
#include "thread_pool.hpp"

#include <memory>
#include <spdlog/logger.h>

#ifdef USE_BOOST_ASIO
#include <boost/asio.hpp>
using namespace boost;
#else
#include <asio.hpp>
#endif

namespace routine {

  class Scheduler : public std::enable_shared_from_this<Scheduler>, private spdlog::logger {
  public:
    Scheduler();
    ~Scheduler() = default;

    void set_router(std::unique_ptr<http::RouteHandler> router);

    void set_io_timeout(size_t milliseconds);
    size_t get_io_timeout() const;

    asio::io_context& get_context();

    void run(size_t io_bound_threads, size_t cpu_bound_threads);

    void join_threads();

    routine::http::RequestHandler_ptr route_request(http::Request_ptr request);
    void prepare_task(std::function<void()> lambda);

  private:
    asio::io_context context_;
    std::unique_ptr<http::RouteHandler> router_;
    ThreadPool cpu_thread_pool_;
    ThreadPool io_thread_pool_;

    size_t io_timeout_ms_;
  };

  using Scheduler_ptr = std::shared_ptr<Scheduler>;

} // namespace routine
