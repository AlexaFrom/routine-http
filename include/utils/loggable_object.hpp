#pragma once

#include <memory>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>

namespace routine::utils {
  inline std::shared_ptr<spdlog::logger> make_logger(std::string name,
                                                     spdlog::level::level_enum level,
                                                     const std::vector<spdlog::sink_ptr>& sinks) {
    auto logger = std::make_shared<spdlog::logger>(name, sinks.begin(), sinks.end());
    logger->set_level(level);
    spdlog::register_logger(logger);

    return std::move(logger);
  }

  class LoggableObject {
  public:
    LoggableObject(std::shared_ptr<spdlog::logger> logger) : logger_(logger) {
      if (!logger_) {
        logger_ = spdlog::get("System");
        logger_->warn("Logger pointer is Nullptr. Using default 'System' logger");
      }
    }
    LoggableObject(std::string name, spdlog::level::level_enum level = spdlog::level::trace)
        : logger_(spdlog::get(name)) {
      if (!logger_) {
        logger_ = spdlog::get("System");
        logger_->warn("Logger '{}' not found. Using default 'System' logger", name);
      } else {
        logger_->set_level(level);
      }
    }

    spdlog::level::level_enum get_level() { return logger_->level(); }
    void set_level(spdlog::level::level_enum level) { logger_->set_level(level); }
    void flush() { logger_->flush(); }

    template <typename... Args>
    void trace(const Args&... args) const {
      logger_->trace(args...);
    }

    template <typename... Args>
    void debug(const Args&... args) const {
      logger_->debug(args...);
    }

    template <typename... Args>
    void info(const Args&... args) const {
      logger_->info(args...);
    }

    template <typename... Args>
    void warn(const Args&... args) const {
      logger_->warn(args...);
    }

    template <typename... Args>
    void error(const Args&... args) const {
      logger_->error(args...);
    }

    template <typename... Args>
    void critical(const Args&... args) const {
      logger_->critical(args...);
    }

  private:
    std::shared_ptr<spdlog::logger> logger_;
  };

}; // namespace routine::utils
