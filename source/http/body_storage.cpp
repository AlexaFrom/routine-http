#include "http/body_storage.hpp"
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
