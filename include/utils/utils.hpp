#pragma once

#include "http/types.hpp"
#include <algorithm>
#include <numeric>
#include <spdlog/spdlog.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace routine::utils {
  template <typename StringT,
            typename = std::enable_if_t<std::is_same_v<std::decay_t<StringT>, std::string> ||
                                        std::is_same_v<std::decay_t<StringT>, std::string_view>>>
  inline std::vector<StringT> split_string(const std::string& input, char delim = ' ',
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
  inline std::vector<StringT> split_string_limit(const std::string& input, char delim,
                                                 size_t limit) {
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
    auto parts = split_string<std::string>(path, '/');
    path = std::accumulate(parts.begin(), parts.end(), std::string{},
                           [](auto a, auto& b) { return a.empty() ? b : a + '/' + b; });
    return std::move(path);
  }
} // namespace routine::utils

namespace routine::http::utils {
  // Make HTTP format datatime
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

  // Convert std::string to routine::http::Version
  inline http::Version version_from_string(const std::string& string) {
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

  // Convert routine::http::Version to std::string_view
  inline std::string_view to_string(http::Version version) {
    static const std::unordered_map<http::Version, std::string> map{
        {http::Version::None, "HTTP/?"},
        {http::Version::Http10, "HTTP/1.0"},
        {http::Version::Http11, "HTTP/1.1"},
        {http::Version::Http2, "HTTP/2"},
        {http::Version::Http3, "HTTP/3"}};
    return map.at(version);
  }

  // Convert std::string to routine::http::Method
  inline http::Method method_from_string(const std::string& string) {
    // std::transform(string.begin(), string.end(), string.begin(), ::toupper);

    static const std::unordered_map<std::string, http::Method> map{
        {"GET", Method::Get},         {"POST", Method::Post},       {"PUT", Method::Put},
        {"DELETE", Method::Delete},   {"PATCH", Method::Patch},     {"HEAD", Method::Head},
        {"OPTIONS", Method::Options}, {"CONNECT", Method::Connect}, {"TRACE", Method::Trace}};

    if (auto it = map.find(string); it != map.end()) { return it->second; }
    return Method::None;
  }

  // Convert routine::http::Method to std::string_view
  inline std::string_view to_string(http::Method method) {
    static const std::array<std::string, 10> names{"NONE",   "GET",  "POST",    "PUT",     "PATCH",
                                                   "DELETE", "HEAD", "OPTIONS", "CONNECT", "TRACE"};

    return names[static_cast<size_t>(method)];
  }

  // Convert routine::http::Status to std::string_view
  inline std::string_view to_string(http::Status status) {
    static const std::unordered_map<Status, std::string> map{
        {Status::Continue, "100 Continue"},
        {Status::Switching_Protocols, "101 Switching_Protocols"},
        {Status::Processing, "102 Processing"},
        {Status::Early_Hints, "103 Early_Hints"},
        {Status::Ok, "200 Ok"},
        {Status::Created, "201 Created"},
        {Status::Accepted, "202 Accepted"},
        {Status::Non_Authoritative_Information, "203 Non_Authoritative_Information"},
        {Status::No_Content, "204 No_Content"},
        {Status::Reset_Content, "205 Reset_Content"},
        {Status::Partial_Content, "206 Partial_Content"},
        {Status::Multi_Status, "207 Multi_Status"},
        {Status::Already_Reported, "208 Already_Reported"},
        {Status::Im_Used, "226 Im_Used"},
        {Status::Multiple_Choices, "300 Multiple_Choices"},
        {Status::Moved_Permanently, "301 Moved_Permanently"},
        {Status::Found, "302 Found"},
        {Status::See_Other, "303 See_Other"},
        {Status::Not_Modified, "304 Not_Modified"},
        {Status::Use_Proxy, "305 Use_Proxy"},
        {Status::Temporary_Redirect, "307 Temporary_Redirect"},
        {Status::Permanent_Redirect, "308 Permanent_Redirect"},
        {Status::Bad_Request, "400 Bad_Request"},
        {Status::Unauthorized, "401 Unauthorized"},
        {Status::Payment_Required, "402 Payment_Required"},
        {Status::Forbidden, "403 Forbidden"},
        {Status::Not_Found, "404 Not_Found"},
        {Status::Method_Not_Allowed, "405 Method_Not_Allowed"},
        {Status::Not_Acceptable, "406 Not_Acceptable"},
        {Status::Proxy_Auth_Required, "407 Proxy_Auth_Required"},
        {Status::Request_Timeout, "408 Request_Timeout"},
        {Status::Conflict, "409 Conflict"},
        {Status::Gone, "410 Gone"},
        {Status::Length_Required, "411 Length_Required"},
        {Status::Precondition_Failed, "412 Precondition_Failed"},
        {Status::Payload_Too_Large, "413 Payload_Too_Large"},
        {Status::Uri_Too_Long, "414 Uri_Too_Long"},
        {Status::Unsupported_Media_Type, "415 Unsupported_Media_Type"},
        {Status::Range_Not_Satisfiable, "416 Range_Not_Satisfiable"},
        {Status::Expectation_Failed, "417 Expectation_Failed"},
        {Status::Misdirected_Request, "421 Misdirected_Request"},
        {Status::Unprocessable_Entity, "422 Unprocessable_Entity"},
        {Status::Locked, "423 Locked"},
        {Status::Failed_Dependency, "424 Failed_Dependency"},
        {Status::Too_Early, "425 Too_Early"},
        {Status::Upgrade_Required, "426 Upgrade_Required"},
        {Status::Precondition_Required, "428 Precondition_Required"},
        {Status::Too_Many_Requests, "429 Too_Many_Requests"},
        {Status::Request_Header_Fields_Too_Large, "431 Request_Header_Fields_Too_Large"},
        {Status::Unavailable_For_Legal_Reasons, "451 Unavailable_For_Legal_Reasons"},
        {Status::Internal_Server_Error, "500 Internal_Server_Error"},
        {Status::Not_Implemented, "501 Not_Implemented"},
        {Status::Bad_Gateway, "502 Bad_Gateway"},
        {Status::Service_Unavailable, "503 Service_Unavailable"},
        {Status::Gateway_Timeout, "504 Gateway_Timeout"},
        {Status::Http_Version_Not_Supported, "505 Http_Version_Not_Supported"},
        {Status::Variant_Also_Negotiates, "506 Variant_Also_Negotiates"},
        {Status::Insufficient_Storage, "507 Insufficient_Storage"},
        {Status::Loop_Detected, "508 Loop_Detected"},
        {Status::Not_Extended, "510 Not_Extended"},
        {Status::Network_Auth_Required, "511 Network_Auth_Required"},
        {Status::None, "0 None"},
    };
    return map.at(status);
  }
}; // namespace routine::http::utils
