#include "scheduler.hpp"
#include <spdlog/spdlog.h>
#include <thread>

routine::Scheduler::Scheduler() : LoggableObject("Scheduler"), context_(), io_timeout_ms_(5000) {}

void routine::Scheduler::set_router(std::unique_ptr<routine::http::RouteHandler> router) {
  router_ = std::move(router);
}

asio::io_context& routine::Scheduler::get_context() {
  return context_;
}

void routine::Scheduler::set_io_timeout(size_t milliseconds) {
  io_timeout_ms_ = milliseconds;
}
size_t routine::Scheduler::get_io_timeout() const {
  return io_timeout_ms_;
}

void routine::Scheduler::run(size_t io_bound_threads, size_t cpu_bound_threads) {
  trace("Running {} IO threads...", io_bound_threads);
  io_thread_pool_.run(io_bound_threads);
  trace("Placing infinite tasks for asio::io_context::run()");

  while (io_bound_threads-- > 0)
    io_thread_pool_.push([self = shared_from_this()]() {
      while (true) {
        self->get_context().run();
      }
    });

  trace("Running {} CPU threads...", cpu_bound_threads);
  cpu_thread_pool_.run(cpu_bound_threads);
}

void routine::Scheduler::join_threads() {
  io_thread_pool_.join();
  cpu_thread_pool_.join();
}

routine::http::RequestHandler_ptr routine::Scheduler::route_request(http::Request_ptr request) {
  if (!request || !router_) return nullptr;
  return router_->route(*request);
}

void routine::Scheduler::prepare_task(std::function<void()> lambda) {
  cpu_thread_pool_.push(std::move(lambda));
}
