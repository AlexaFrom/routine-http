#pragma once

#include <chrono>
#include <functional>
#include <spdlog/spdlog.h>

namespace routine::utils {

  inline void benchmark(const std::string& title, std::function<void()> lambda,
                        size_t iters = 1000) {
    auto start = std::chrono::steady_clock::now();
    for (size_t i = 0; i < iters; ++i)
      lambda();
    auto end = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    double seconds = elapsed.count() / 1000.0;
    double rps = seconds > 0 ? iters / seconds : 0;
    spdlog::get("Benchmark")->info("{} # Total elapsed {}. {:.1f} rps", title, elapsed, rps);
  }

}; // namespace routine::utils
