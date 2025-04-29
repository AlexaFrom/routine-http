#include "http/body_storage.hpp"
#include <boost/asio/buffers_iterator.hpp>
#include <cstring>

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

void routine::http::JsonBody::write(const std::vector<uint8_t>& buffer) {
  data_ = nlohmann::json::parse(std::string(buffer.begin(), buffer.end()));
}

void routine::http::JsonBody::write(asio::streambuf& buffer) {
  data_ = nlohmann::json::parse(
      std::string(asio::buffers_begin(buffer.data()), asio::buffers_end(buffer.data())));
}

std::vector<uint8_t> routine::http::JsonBody::read() const {
  std::string str = data_.dump();
  return std::vector<uint8_t>(str.begin(), str.end());
}

size_t routine::http::JsonBody::size() const {
  return data_.size();
}

std::string routine::http::JsonBody::as_string() const {
  return data_.dump();
}

const uint8_t* routine::http::JsonBody::data() const {
  return nullptr;
}

const nlohmann::json& routine::http::JsonBody::json() const {
  return data_;
}
