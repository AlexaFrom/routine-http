#pragma once

#include "http/types.hpp"
#include <algorithm>
#include <numeric>
#include <spdlog/spdlog.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace routine::utils {
  template <typename StringT,
            typename = std::enable_if_t<std::is_same_v<std::decay_t<StringT>, std::string> ||
                                        std::is_same_v<std::decay_t<StringT>, std::string_view>>>
  inline std::vector<StringT> splitString(const std::string& input, char delim = ' ',
                                          size_t reserve_size = 4) {
    std::vector<StringT> result;
    result.reserve(reserve_size);
    std::istringstream ss(input);
    std::string token;
    while (std::getline(ss, token, delim)) {
      if (token.empty()) continue;
      result.push_back(token);
    }
    return result;
  }

  template <typename StringT,
            typename = std::enable_if_t<std::is_same_v<std::decay_t<StringT>, std::string> ||
                                        std::is_same_v<std::decay_t<StringT>, std::string_view>>>
  inline std::vector<StringT> splitStringLimit(const std::string& input, char delim, size_t limit) {
    std::vector<StringT> result;
    result.reserve(limit);
    std::istringstream ss(input);
    std::string token;
    while (limit-- > 0 && std::getline(ss, token, delim)) {
      if (token.empty()) continue;
      result.push_back(token);
    }
    return result;
  }

  inline std::string format_path(std::string path) {
    auto parts = splitString<std::string>(path, '/');
    path = std::accumulate(parts.begin(), parts.end(), std::string{},
                           [](auto a, auto& b) { return a.empty() ? b : a + '/' + b; });
    return std::move(path);
  }
} // namespace routine::utils

namespace routine::http::utils {
  inline std::string get_current_http_date() {
    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);

    std::tm tm_gmt;
    gmtime_r(&time, &tm_gmt);

    // RFC 7231
    std::ostringstream oss;
    oss << std::put_time(&tm_gmt, "%a, %d %b %Y %H:%M:%S GMT");

    return oss.str();
  }

  inline http::Version getVersionFromString(std::string string) {
    // std::transform(string.begin(), string.end(), string.begin(), ::toupper);
    if (string.starts_with("HTTP/1.0"))
      return Version::Http10;
    else if (string.starts_with("HTTP/1.1"))
      return Version::Http11;
    else if (string.starts_with("HTTP/2"))
      return Version::Http2;
    else if (string.starts_with("HTTP/3"))
      return Version::Http3;
    return Version::None;
  }

  // Преобразует строку (например, "GET") в enum Method
  inline http::Method getMethodFromString(std::string string) {
    // std::transform(string.begin(), string.end(), string.begin(), ::toupper);

    static const std::unordered_map<std::string, http::Method> map = {
        {"GET", Method::Get},         {"POST", Method::Post},       {"PUT", Method::Put},
        {"DELETE", Method::Delete},   {"PATCH", Method::Patch},     {"HEAD", Method::Head},
        {"OPTIONS", Method::Options}, {"CONNECT", Method::Connect}, {"TRACE", Method::Trace}};

    if (auto it = map.find(string); it != map.end()) { return it->second; }
    return Method::None;
  }

  // Преобразует enum Method в строку (например, Method::Get -> "GET")
  inline std::string getMethodByString(http::Method method) {
    static const std::array<std::string, 10> names = {
        "NONE", "GET", "POST", "PUT", "PATCH", "DELETE", "HEAD", "OPTIONS", "CONNECT", "TRACE"};

    size_t index = static_cast<size_t>(static_cast<uint8_t>(method));
    return names[index];
  }

  inline std::string getStatusAsString(http::Status status) {
    static std::unordered_map<Status, std::string> map{
        {Status::None, "Unknown Status"},
        // 1xx Informational
        {Status::CONTINUE, "Continue"},
        {Status::SWITCHING_PROTOCOLS, "Switching Protocols"},
        {Status::PROCESSING, "Processing"},
        {Status::EARLY_HINTS, "Early Hints"},

        // 2xx Success
        {Status::OK, "OK"},
        {Status::CREATED, "Created"},
        {Status::ACCEPTED, "Accepted"},
        {Status::NON_AUTHORITATIVE_INFORMATION, "Non-Authoritative Information"},
        {Status::NO_CONTENT, "No Content"},
        {Status::RESET_CONTENT, "Reset Content"},
        {Status::PARTIAL_CONTENT, "Partial Content"},
        {Status::MULTI_STATUS, "Multi-Status"},
        {Status::ALREADY_REPORTED, "Already Reported"},
        {Status::IM_USED, "IM Used"},

        // 3xx Redirection
        {Status::MULTIPLE_CHOICES, "Multiple Choices"},
        {Status::MOVED_PERMANENTLY, "Moved Permanently"},
        {Status::FOUND, "Found"},
        {Status::SEE_OTHER, "See Other"},
        {Status::NOT_MODIFIED, "Not Modified"},
        {Status::USE_PROXY, "Use Proxy"},
        {Status::TEMPORARY_REDIRECT, "Temporary Redirect"},
        {Status::PERMANENT_REDIRECT, "Permanent Redirect"},

        // 4xx Client Errors
        {Status::BAD_REQUEST, "Bad Request"},
        {Status::UNAUTHORIZED, "Unauthorized"},
        {Status::PAYMENT_REQUIRED, "Payment Required"},
        {Status::FORBIDDEN, "Forbidden"},
        {Status::NOT_FOUND, "Not Found"},
        {Status::METHOD_NOT_ALLOWED, "Method Not Allowed"},
        {Status::NOT_ACCEPTABLE, "Not Acceptable"},
        {Status::PROXY_AUTH_REQUIRED, "Proxy Authentication Required"},
        {Status::REQUEST_TIMEOUT, "Request Timeout"},
        {Status::CONFLICT, "Conflict"},
        {Status::GONE, "Gone"},
        {Status::LENGTH_REQUIRED, "Length Required"},
        {Status::PRECONDITION_FAILED, "Precondition Failed"},
        {Status::PAYLOAD_TOO_LARGE, "Payload Too Large"},
        {Status::URI_TOO_LONG, "URI Too Long"},
        {Status::UNSUPPORTED_MEDIA_TYPE, "Unsupported Media Type"},
        {Status::RANGE_NOT_SATISFIABLE, "Range Not Satisfiable"},
        {Status::EXPECTATION_FAILED, "Expectation Failed"},
        {Status::MISDIRECTED_REQUEST, "Misdirected Request"},
        {Status::UNPROCESSABLE_ENTITY, "Unprocessable Entity"},
        {Status::LOCKED, "Locked"},
        {Status::FAILED_DEPENDENCY, "Failed Dependency"},
        {Status::TOO_EARLY, "Too Early"},
        {Status::UPGRADE_REQUIRED, "Upgrade Required"},
        {Status::PRECONDITION_REQUIRED, "Precondition Required"},
        {Status::TOO_MANY_REQUESTS, "Too Many Requests"},
        {Status::REQUEST_HEADER_FIELDS_TOO_LARGE, "Request Header Fields Too Large"},
        {Status::UNAVAILABLE_FOR_LEGAL_REASONS, "Unavailable For Legal Reasons"},

        // 5xx Server Errors
        {Status::INTERNAL_SERVER_ERROR, "Internal Server Error"},
        {Status::NOT_IMPLEMENTED, "Not Implemented"},
        {Status::BAD_GATEWAY, "Bad Gateway"},
        {Status::SERVICE_UNAVAILABLE, "Service Unavailable"},
        {Status::GATEWAY_TIMEOUT, "Gateway Timeout"},
        {Status::HTTP_VERSION_NOT_SUPPORTED, "HTTP Version Not Supported"},
        {Status::VARIANT_ALSO_NEGOTIATES, "Variant Also Negotiates"},
        {Status::INSUFFICIENT_STORAGE, "Insufficient Storage"},
        {Status::LOOP_DETECTED, "Loop Detected"},
        {Status::NOT_EXTENDED, "Not Extended"},
        {Status::NETWORK_AUTH_REQUIRED, "Network Authentication Required"}};
    return std::format("{} {}", static_cast<uint16_t>(status), map.at(status));
  }
}; // namespace routine::http::utils
