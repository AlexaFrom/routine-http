#pragma once

#include <cstdint>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <vector>

#ifdef USE_BOOST_ASIO
#include <boost/asio/streambuf.hpp>
using namespace boost;
#else
#include <asio/streambuf.hpp>
#endif

namespace routine::http {

  enum class StorageType { None, Memory, File, Json };

  class I_BodyStorage {
    // TODO # add move&copy write methods
  public:
    virtual void operator=(const std::string& str) = 0;

    virtual void write(const std::vector<uint8_t>& buffer) = 0;
    virtual void write(asio::streambuf& buffer) = 0;
    virtual std::vector<uint8_t> read() const = 0;
    virtual size_t size() const = 0;
    virtual std::string as_string() const = 0;

    virtual StorageType get_type() const { return StorageType::None; }

    virtual ~I_BodyStorage() = default;
  };

  class MemoryBody : public I_BodyStorage {
  public:
    void operator=(const std::string& str) override;

    void write(const std::vector<uint8_t>& buffer) override;
    void write(asio::streambuf& buffer) override;
    std::vector<uint8_t> read() const override;
    size_t size() const override;
    std::string as_string() const override;

    const uint8_t* data() const;

    StorageType get_type() const override { return StorageType::Memory; }

  private:
    std::vector<uint8_t> data_;
  };

  class FileBody : public I_BodyStorage {
  public:
    void operator=(const std::string& str) override;

    void write(const std::vector<uint8_t>& buffer) override;
    void write(asio::streambuf& buffer) override;
    std::vector<uint8_t> read() const override;
    size_t size() const override;
    std::string as_string() const override;

    const uint8_t* data() const;

    StorageType get_type() const override { return StorageType::File; }

  private:
    std::vector<uint8_t> data_;
  };

  class JsonBody : public I_BodyStorage {
  public:
    void operator=(const std::string& str) override;

    void write(const std::vector<uint8_t>& buffer) override;
    void write(asio::streambuf& buffer) override;
    std::vector<uint8_t> read() const override;
    size_t size() const override;
    std::string as_string() const override;

    const uint8_t* data() const;

    StorageType get_type() const override { return StorageType::Json; }

    const nlohmann::json& json() const;

  private:
    nlohmann::json data_;
  };

} // namespace routine::http
