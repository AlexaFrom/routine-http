#include "thread_pool.hpp"
#include <chrono>
#include <fmt/std.h>
#include <memory>
#include <mutex>
#include <numeric>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <thread>

// TODO # мб добавить флаг о том, что проц в паузе?

routine::ThreadPool::ThreadPool() : spdlog::logger(*spdlog::get("ThreadPool")) {}

void routine::ThreadPool::push(std::function<void()> lambda) {
  if (threads_.empty()) {
    error("No available created threads. Execute ThreadPool::run(size_t n) for create n threads");
    lambda();
    return;
  }

  std::pair<size_t, size_t> least_loaded_worker = {0, threads_[0]->queue_.size()};
  for (size_t i = 0; i < threads_.size(); ++i) {
    if (threads_[i]->queue_.empty()) {
      // если очередь пустая - устанавливаем текущий поток как наиболее свободный
      least_loaded_worker.first = i;
      break;
    }
    if (threads_[i]->queue_.size() < least_loaded_worker.second) {
      // ищем минимально загруженный поток
      least_loaded_worker.first = i;
      least_loaded_worker.second = threads_[i]->queue_.size();
    }
  }

  {
    std::lock_guard<std::mutex> guard(threads_[least_loaded_worker.first]->mutex_);
    threads_[least_loaded_worker.first]->queue_.push(std::move(lambda));
  }
  threads_[least_loaded_worker.first]->cv_.notify_all();
  debug("Lambda pushed to thread #id {}", least_loaded_worker.first);
}

void routine::ThreadPool::run(size_t n) {
  while (n-- > 0)
    threads_.push_back(std::make_unique<CpuThreadWorker>());
}

void routine::ThreadPool::stop(size_t n) {
  info("Stopped {} threads", n);
  std::vector<std::unique_ptr<CpuThreadWorker>> stopped;
  {
    n = std::min(n, threads_.size());

    stopped.reserve(n);

    for (size_t i = 0; i < n; ++i) {
      threads_[i]->stop();
      stopped.push_back(std::move(threads_[i]));
    }

    threads_.erase(threads_.begin(), threads_.begin() + n);
  }

  // переносим оставшиеся задачи в отключенных потоках
  if (threads_.empty()) {
    size_t losses_task =
        std::accumulate(stopped.begin(), stopped.end(), 0ull,
                        [](size_t a, const auto& b) { return a + b->queue_.size(); });
    warn("All threads will be stopped. {} tasks losses", losses_task);
    return;
  }
  for (auto& thread : stopped) {
    std::lock_guard<std::mutex> guard(thread->mutex_);
    while (!thread->queue_.empty()) {
      push(std::move(thread->queue_.front()));
      thread->queue_.pop();
    }
  }
}

void routine::ThreadPool::join() {
  for (auto& thread : threads_)
    if (thread->thread_.joinable()) thread->thread_.join();
}

size_t routine::ThreadPool::tasks_count() const {
  return std::accumulate(threads_.begin(), threads_.end(), 0ull,
                         [](size_t a, const auto& b) { return a + b->queue_.size(); });
}
size_t routine::ThreadPool::threads_count() const {
  return threads_.size();
}

//  //  //  //  //  //  //  //  //  //  //  //  //  //  //  //  //  //  //  //

routine::ThreadPool::CpuThreadWorker::CpuThreadWorker()
    : status_(Status::Working), thread_(&CpuThreadWorker::process_worker, this) {}

routine::ThreadPool::CpuThreadWorker::~CpuThreadWorker() {
  stop();
}

void routine::ThreadPool::CpuThreadWorker::stop() {
  status_ = Status::Stopped;
  if (thread_.joinable()) thread_.join();
}

void routine::ThreadPool::CpuThreadWorker::process_worker() {
  spdlog::get("ThreadPool")->info("Thread #{} started", std::this_thread::get_id());
  while (true) {
    std::unique_lock<std::mutex> ulock(mutex_);
    cv_.wait_for(ulock, std::chrono::milliseconds(100),
                 [this]() { return !queue_.empty() || status_ == Status::Stopped; });

    if (status_ == Status::Stopped) return;
    if (queue_.empty()) continue;

    auto task = std::move(queue_.front());
    queue_.pop();
    ulock.unlock();

    task();
  }
}
