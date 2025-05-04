#include "http/body_storage.hpp"
#include <boost/asio/buffers_iterator.hpp>
#include <cstring>
#include <exception>
#include <iterator>
#include <spdlog/spdlog.h>
#include <tao/json/from_input.hpp>
#include <tao/json/from_stream.hpp>
#include <tao/json/from_string.hpp>

void routine::http::MemoryBody::operator=(const std::string& str) {
  data_.resize(str.size());
  ::memcpy(data_.data(), str.data(), data_.size());
}

void routine::http::MemoryBody::write(const std::vector<uint8_t>& buffer) {
  size_t size = data_.size();
  data_.resize(size + buffer.size());
  ::memcpy(data_.data() + size, buffer.data(), buffer.size());
}

void routine::http::MemoryBody::write(asio::streambuf& buffer) {
  size_t size = data_.size();
  data_.resize(size + buffer.data().size());
  ::memcpy(data_.data() + size, buffer.data().data(), buffer.data().size());
  buffer.consume(buffer.size());
}

void routine::http::MemoryBody::write(const std::string& buffer) {
  size_t size = data_.size();
  data_.resize(size + buffer.size());
  ::memcpy(data_.data() + size, buffer.data(), buffer.size());
}

void routine::http::MemoryBody::write(std::string&& buffer) {
  data_.insert(data_.end(), std::make_move_iterator(buffer.begin()),
               std::make_move_iterator(buffer.end()));
}

std::vector<uint8_t> routine::http::MemoryBody::read() const {
  return data_;
}

size_t routine::http::MemoryBody::size() const {
  return data_.size();
}

std::string routine::http::MemoryBody::as_string() const {
  return std::string(data_.begin(), data_.end());
}

const uint8_t* routine::http::MemoryBody::data() const {
  return data_.data();
}

//  //  //  //  //  //  //  //  //  //  //  //  //  //  //  //  //  //  //  // //  //  //  //  // //

void routine::http::JsonBody::operator=(const std::string& buffer) {
  write(buffer);
}

void routine::http::JsonBody::write(const std::vector<uint8_t>& buffer) {
  write(std::string(buffer.begin(), buffer.end()));
}

void routine::http::JsonBody::write(asio::streambuf& buffer) {
  write(std::string(asio::buffers_begin(buffer.data()), asio::buffers_end(buffer.data())));
}

void routine::http::JsonBody::write(const std::string& buffer) {
  try {
    data_ = tao::json::from_string(buffer);
  } catch (const std::exception& e) {
    spdlog::get("Http")->error("Exception # JsonBody # {}", e.what());
  }
}

void routine::http::JsonBody::write(std::string&& buffer) {
  try {
    data_ = tao::json::from_string(std::move(buffer));
  } catch (const std::exception& e) {
    spdlog::get("Http")->error("Exception # JsonBody # {}", e.what());
  }
}

std::vector<uint8_t> routine::http::JsonBody::read() const {
  std::string str = data_.get_string();
  return std::vector<uint8_t>(str.begin(), str.end());
}

size_t routine::http::JsonBody::size() const {
  return data_.get_string().size();
}

std::string routine::http::JsonBody::as_string() const {
  return data_.get_string();
}

const uint8_t* routine::http::JsonBody::data() const {
  return nullptr;
}

const tao::json::value& routine::http::JsonBody::json() const {
  return data_;
}
