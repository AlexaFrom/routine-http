#pragma once

#include <string>
#include <unordered_map>
#include <utility>
namespace routine::http {

  class Request;
  class Response;

  class Headers {
  public:
    using key_type = std::string;
    using value_type = std::string;
    using container = std::unordered_map<key_type, value_type>;
    using iterator = container::iterator;
    using const_iterator = container::const_iterator;

  public:
    Headers() = default;
    Headers(const std::string& raw);
    Headers(std::istringstream& stream);
    Headers(std::initializer_list<std::pair<const key_type, value_type>> initializer_list);

    std::string& operator[](const std::string& key) const;
    std::string& at(const std::string& key) const;

    bool contains(const std::string& key) const noexcept;

    void insert(key_type&& key, value_type&& value);

    void insert(std::pair<key_type, value_type>&& pair);

    void clear() noexcept;

    iterator begin() noexcept;
    iterator end() noexcept;

    const_iterator begin() const noexcept;
    const_iterator end() const noexcept;

    const_iterator cbegin() const noexcept;
    const_iterator cend() const noexcept;

  private:
    void init_from_stream(std::istringstream& stream);

  private:
    mutable container headers_;

    friend http::Request;
    friend http::Response;
  };

} // namespace routine::http
