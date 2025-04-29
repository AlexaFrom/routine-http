#pragma once

#include "http/request.hpp"
#include "request_handler.hpp"
#include "utils/loggable_object.hpp"
#include <condition_variable>
#include <cstdint>
#include <queue>
#include <thread>

namespace routine {

  class ThreadPool : utils::LoggableObject {

    enum class Status : uint8_t { Working, Stopped };

    struct CpuThreadWorker {
      CpuThreadWorker();
      ~CpuThreadWorker();

      void process_worker();

      void stop();

      std::thread thread_;

      std::queue<std::function<void()>> queue_;
      std::mutex mutex_;
      std::condition_variable cv_;
      std::atomic<Status> status_;
    };

  public:
    ThreadPool();

    void run(size_t n);
    void stop(size_t n);
    void join();

    size_t tasks_count() const;
    size_t threads_count() const;

    void push(std::function<void()> lambda);

  private:
    std::vector<std::unique_ptr<CpuThreadWorker>> threads_;
  };

} // namespace routine
