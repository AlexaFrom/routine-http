#include "http/headers.hpp"
#include "http/types.hpp"
#include "utils/utils.hpp"
#include <format>
#include <spdlog/spdlog.h>
#include <sstream>
#include <stdexcept>
#include <string_view>

routine::http::Headers::Headers(const std::string& input) {
  std::istringstream stream(input);
  init_from_stream(stream);
}

routine::http::Headers::Headers(std::istringstream& stream) {
  init_from_stream(stream);
}

void routine::http::Headers::init_from_stream(std::istringstream& stream) {
  std::string line;
  while (std::getline(stream, line)) {
    if (line[0] == '\r') break;

    size_t n = line.find(':', 2);
    if (n == std::string::npos) continue;

    std::string header = line.substr(0, n);
    std::string value =
        line.substr(n + 2, line.size() - n - 3); // skip ": " and '\r' at back of line

    std::transform(header.begin(), header.end(), header.begin(), ::tolower);

    headers_.emplace(std::move(header), std::move(value));
  }
}

const routine::http::HeaderField& routine::http::Headers::at(std::string_view key) const {
  auto it = headers_.find(key);
  if (it == headers_.end()) throw std::invalid_argument(std::format("Header '{}' not found", key));
  return *it;
}

const routine::http::HeaderField& routine::http::Headers::at(std::string_view key) noexcept {
  auto it = headers_.find(key);
  if (it == headers_.end()) it = headers_.emplace(key.data()).first;
  return *it;
}

const routine::http::HeaderField& routine::http::Headers::operator[](std::string_view key) const {
  auto it = headers_.find(key);
  if (it == headers_.end()) throw std::invalid_argument(std::format("Header '{}' not found", key));
  return *it;
}

const routine::http::HeaderField&
routine::http::Headers::operator[](std::string_view key) noexcept {
  auto it = headers_.find(key);
  if (it == headers_.end()) it = headers_.emplace(key.data()).first;
  return *it;
}

const routine::http::HeaderField& routine::http::Headers::at(routine::http::Header key) const {
  return at(utils::to_string(key));
}

const routine::http::HeaderField& routine::http::Headers::at(routine::http::Header key) noexcept {
  return at(utils::to_string(key));
}

const routine::http::HeaderField&
routine::http::Headers::operator[](routine::http::Header key) const {
  return at(utils::to_string(key));
}

const routine::http::HeaderField&
routine::http::Headers::operator[](routine::http::Header key) noexcept {
  return at(utils::to_string(key));
}

bool routine::http::Headers::contains(std::string_view key) const noexcept {
  return headers_.contains(key);
}

bool routine::http::Headers::contains(routine::http::Header key) const noexcept {
  return headers_.contains(utils::to_string(key));
}

void routine::http::Headers::insert(
    std::pair<HeaderField::key_type, HeaderField::value_type>&& pair) {
  insert(std::forward<HeaderField::key_type>(pair.first),
         std::forward<HeaderField::value_type>(pair.second));
}

void routine::http::Headers::insert(HeaderField&& header) {
  headers_.insert(std::move(header));
}

void routine::http::Headers::insert(const HeaderField& header) {
  headers_.insert(header);
}

void routine::http::Headers::clear() noexcept {
  headers_.clear();
}

routine::http::Headers::container::iterator routine::http::Headers::begin() noexcept {
  return headers_.begin();
}
routine::http::Headers::container::iterator routine::http::Headers::end() noexcept {
  return headers_.end();
}

routine::http::Headers::container::const_iterator routine::http::Headers::begin() const noexcept {
  return headers_.begin();
}
routine::http::Headers::container::const_iterator routine::http::Headers::end() const noexcept {
  return headers_.end();
}

routine::http::Headers::container::const_iterator routine::http::Headers::cbegin() const noexcept {
  return headers_.cbegin();
}
routine::http::Headers::container::const_iterator routine::http::Headers::cend() const noexcept {
  return headers_.cend();
}
