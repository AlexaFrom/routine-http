#pragma once

#include "http/types.hpp"
#include "utils/utils.hpp"
#include <concepts>
#include <string>
#include <unordered_set>

namespace routine::http {
  class Request;
  class Response;

  class HeaderField {
  public:
    using key_type = std::string;
    using value_type = std::string;

    template <typename KeyT>
      requires std::constructible_from<key_type, KeyT>
    HeaderField(KeyT&& key) : key_(std::forward<KeyT>(key)), value_("") {}

    template <typename KeyT, typename ValueT>
      requires std::constructible_from<key_type, KeyT> &&
                   std::constructible_from<value_type, ValueT>
    HeaderField(KeyT&& key, ValueT&& value)
        : key_(std::forward<KeyT>(key)), value_(std::forward<ValueT>(value)) {}

    template <typename ValueT>
      requires std::constructible_from<value_type, ValueT>
    HeaderField(routine::http::Header key, ValueT&& value)
        : key_(routine::http::utils::to_string(key)), value_(std::forward<ValueT>(value)) {}

    HeaderField(HeaderField&&) noexcept = default;
    HeaderField& operator=(HeaderField&&) noexcept = default;
    HeaderField(const HeaderField&) = default;

  public:
    // Compare both 'key' fields of HeaderField
    bool operator==(const HeaderField& other) const { return other.value() == this->value(); }

    // Compare ValueT with HeaderField::value()
    template <typename ValueT>
      requires std::constructible_from<value_type, ValueT>
    bool operator==(ValueT&& other) const {
      return other == this->value();
    }

    // Assign ValueT to HeaderField::value()
    template <typename ValueT>
      requires std::constructible_from<value_type, ValueT>
    const HeaderField& operator=(ValueT&& value) const {
      value_ = std::forward<ValueT>(value);
      return *this;
    }

    operator std::string() const { return value_; }

  public:
    // Get Header name
    const std::string& key() const { return key_; }
    // Get value
    std::string& value() const { return value_; }

    // Get string as format "Key: Value"
    std::string as_string() const { return std::format("{}: {}", key_, value_); }

    // Assign new value to HeaderField::value()
    template <typename ValueT>
      requires std::constructible_from<value_type, ValueT>
    void set_value(ValueT&& value) const {
      value_ = std::forward<ValueT>(value);
    }

  private:
    key_type key_;
    mutable value_type value_;
  };

  namespace utils {

    struct HeaderFieldHash {
      using is_transparent = void;

      size_t operator()(const routine::http::HeaderField& object) const {
        return std::hash<std::string_view>{}(object.key());
      }

      size_t operator()(std::string_view key) const { return std::hash<std::string_view>{}(key); }
    };

    struct HeaderFieldEqual {
      using is_transparent = void;

      bool operator()(const HeaderField& left, const HeaderField& right) const {
        return left.key() == right.key();
      }

      bool operator()(const HeaderField& left, std::string right) const {
        return left.key() == right;
      }

      bool operator()(std::string left, const HeaderField& right) const {
        return left == right.key();
      }
    };
  } // namespace utils
} // namespace routine::http

namespace routine::http {

  class Headers {
  public:
    using container =
        std::unordered_set<HeaderField, utils::HeaderFieldHash, utils::HeaderFieldEqual>;

  public:
    Headers() = default;
    Headers(const std::string& raw);
    Headers(std::istringstream& stream);

    // return the HeaderField& or throw exception if key is not found
    const HeaderField& at(std::string_view key) const;

    // return the HeaderField& or create new HeaderField and return it
    const HeaderField& at(std::string_view key) noexcept;

    // return the HeaderField& or throw exception if key is not found
    const HeaderField& operator[](std::string_view key) const;

    // return the HeaderField& or create new HeaderField and return it
    const HeaderField& operator[](std::string_view key) noexcept;

    // return the HeaderField& or throw exception if key is not found
    const HeaderField& at(routine::http::Header key) const;

    // return the HeaderField& or create new HeaderField and return it
    const HeaderField& at(routine::http::Header key) noexcept;

    // return the HeaderField& or throw exception if key is not found
    const HeaderField& operator[](routine::http::Header key) const;

    // return the HeaderField& or create new HeaderField and return it
    const HeaderField& operator[](routine::http::Header key) noexcept;

    bool contains(std::string_view key) const noexcept;

    bool contains(routine::http::Header key) const noexcept;

    void insert(HeaderField&& header);
    void insert(const HeaderField& header);

    template <typename KeyT, typename ValueT>
    void insert(KeyT&& key, ValueT&& value) {
      headers_.emplace(std::forward<HeaderField::key_type>(key),
                       std::forward<HeaderField::value_type>(value));
    }

    void insert(std::pair<HeaderField::key_type, HeaderField::value_type>&& pair);

    void clear() noexcept;

    container::iterator begin() noexcept;
    container::iterator end() noexcept;

    container::const_iterator begin() const noexcept;
    container::const_iterator end() const noexcept;

    container::const_iterator cbegin() const noexcept;
    container::const_iterator cend() const noexcept;

  private:
    void init_from_stream(std::istringstream& stream);

  private:
    container headers_;

    friend http::Request;
    friend http::Response;
  };

} // namespace routine::http
