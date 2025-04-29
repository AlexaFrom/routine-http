#pragma once

#include <charconv>
#include <optional>
#include <string>
#include <string_view>
#include <strings.h>
#include <type_traits>
#include <unordered_map>

namespace routine::http { // namespace utils

  class Parameters {
  public:
    class Field {
    public:
      Field() = default;
      explicit Field(std::string value) : value_(std::move(value)) {}

      Field(Field&&) = default;
      Field(const Field&) = default;

      std::string_view value() const { return value_; }

      template <typename T>
      std::optional<T> as() const noexcept {
        if constexpr (std::is_same_v<T, std::string>) {
          return value_;
        } else if constexpr (std::is_same_v<T, std::string_view>) {
          return value_;
        } else if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
          return parse_integer<T>();
        } else if constexpr (std::is_floating_point_v<T>) {
          return parse_floating<T>();
        } else if constexpr (std::is_same_v<T, bool>) {
          return parse_boolean();
        } else {
          static_assert(!sizeof(T), "Unsupported type conversion!");
        }
      }

    private:
      std::string value_;

    private:
      template <typename T>
      std::optional<T> parse_integer() const noexcept {
        T result;
        auto [ptr, ec] = std::from_chars(value_.data(), value_.data() + value_.size(), result);

        if (ec != std::errc() || ptr != value_.data() + value_.size()) return std::nullopt;
        return result;
      }

      template <typename T>
      std::optional<T> parse_floating() const noexcept {
        T result;
        auto [ptr, ec] = std::from_chars(value_.data(), value_.data() + value_.size(), result);

        if (ec != std::errc() || ptr != value_.data() + value_.size()) return std::nullopt;
        return result;
      }

      std::optional<bool> parse_boolean() const noexcept {
        if (value_.empty()) return std::nullopt;

        if (value_.size() == 1) {
          if (value_[0] == '1') return true;
          if (value_[0] == '0') return false;
          return std::nullopt;
        }

        if (value_.size() == 4) {
          if (strncasecmp(value_.data(), "true", 4) == 0) return true;
        } else if (value_.size() == 5) {
          if (strncasecmp(value_.data(), "false", 5) == 0) return false;
        }

        return std::nullopt;
      }
    };

  public:
    bool contains(const std::string& key) const noexcept { return params_.contains(key); }

    size_t size() const noexcept { return params_.size(); }

    void emplace(std::string key, std::string value) noexcept {
      params_.emplace(std::move(key), std::move(value));
    }

    Field& at(const std::string& key) { return params_.at(key); }

    Field& operator[](const std::string& key) { return params_[key]; }

    void erase(const std::string& key) { params_.erase(key); }

    auto begin() noexcept { return params_.begin(); }
    auto end() noexcept { return params_.end(); }
    auto begin() const noexcept { return params_.begin(); }
    auto end() const noexcept { return params_.end(); }

  private:
    std::unordered_map<std::string, Field> params_;
  };

} // namespace routine::http
