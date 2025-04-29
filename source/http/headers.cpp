#include "http/headers.hpp"
#include <sstream>
#include <utility>

routine::http::Headers::Headers(const std::string& input) {
  std::istringstream stream(input);
  init_from_stream(stream);
}

routine::http::Headers::Headers(std::istringstream& stream) {
  init_from_stream(stream);
}

routine::http::Headers::Headers(
    std::initializer_list<std::pair<const key_type, value_type>> initializer_list)
    : headers_(initializer_list) {}

void routine::http::Headers::init_from_stream(std::istringstream& stream) {
  std::string line;
  while (std::getline(stream, line)) {
    if (line.starts_with("\r\n\r\n")) break;

    size_t n = line.find(':', 2);
    if (n == std::string::npos) continue;

    std::string header = line.substr(0, n);
    std::string value =
        line.substr(n + 2, line.size() - n - 3); // skip ": " and '\r' at back of line

    std::transform(header.begin(), header.end(), header.begin(), ::tolower);

    headers_.try_emplace(header, value);
  }
}

std::string& routine::http::Headers::at(const std::string& key) const {
  return headers_.at(key);
}

std::string& routine::http::Headers::operator[](const std::string& key) const {
  return headers_[key];
}

bool routine::http::Headers::contains(const std::string& key) const noexcept {
  return headers_.contains(key);
}

void routine::http::Headers::insert(key_type&& key, value_type&& value) {
  headers_.emplace(std::forward<key_type>(key), std::forward<value_type>(value));
}

void routine::http::Headers::insert(std::pair<key_type, value_type>&& pair) {
  insert(std::forward<key_type>(pair.first), std::forward<value_type>(pair.second));
}

void routine::http::Headers::clear() noexcept {
  headers_.clear();
}

routine::http::Headers::iterator routine::http::Headers::begin() noexcept {
  return headers_.begin();
}
routine::http::Headers::iterator routine::http::Headers::end() noexcept {
  return headers_.end();
}

routine::http::Headers::const_iterator routine::http::Headers::begin() const noexcept {
  return headers_.begin();
}
routine::http::Headers::const_iterator routine::http::Headers::end() const noexcept {
  return headers_.end();
}

routine::http::Headers::const_iterator routine::http::Headers::cbegin() const noexcept {
  return headers_.cbegin();
}
routine::http::Headers::const_iterator routine::http::Headers::cend() const noexcept {
  return headers_.cend();
}
