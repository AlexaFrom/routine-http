#include "http/request.hpp"
#include "utils/utils.hpp"
#include <algorithm>
#include <spdlog/spdlog.h>
#include <sstream>

routine::http::Request::Request(const std::string& raw_http) {
  std::istringstream stream(raw_http);
  std::string line;
  std::getline(stream, line);

  // parse first line
  {
    auto parts = routine::utils::split_string_limit<std::string>(line, ' ', 3);
    method_ = utils::method_from_string(std::move(parts[0]));

    size_t path_query_delimiter_index = parse_query_parameters(parts[1]);
    if (path_query_delimiter_index == 0) {
      path_ = routine::utils::format_path(std::move(parts[1]));
    } else {
      path_ = parts[1].substr(0, path_query_delimiter_index);
    }

    version_ = utils::version_from_string(std::move(parts[2]));
  }

  // parse headers
  headers_.init_from_stream(stream);
}

size_t routine::http::Request::parse_query_parameters(const std::string& string) {
  auto query_params_index = string.find('?');
  if (query_params_index == std::string::npos) return 0;

  const auto lambda_split_parameter =
      [](const std::string& str) -> std::pair<std::string, std::string> {
    size_t ind = str.find('=');
    if (ind == std::string::npos)
      return {str, ""};
    else
      return {str.substr(0, ind), str.substr(ind + 1, str.size())};
  };

  path_ = string.substr(0, query_params_index);
  auto params = routine::utils::split_string<std::string>(
      string.substr(query_params_index + 1, string.size()), '&');
  std::for_each(params.begin(), params.end(),
                [lambda_split_parameter, this](const std::string& str) {
                  auto [key, value] = lambda_split_parameter(str);
                  query_params_.emplace(std::move(key), std::move(value));
                });
  return query_params_index;
}

const routine::http::Headers& routine::http::Request::headers() {
  return headers_;
}
routine::http::Method routine::http::Request::method() {
  return method_;
}
const std::string& routine::http::Request::path() {
  return path_;
}
routine::http::Version routine::http::Request::version() {
  return version_;
}

std::string routine::http::Request::prepare_request() const {
  std::ostringstream stream;
  stream << utils::to_string(method_) << " /";

  if (path_params_.size() > 0) {
    auto path_parts = routine::utils::split_string<std::string>(path_, '/');
    for (auto& part : path_parts) {
      if (part[0] == '{')
        stream << path_params_[part.substr(1, part.size() - 2)].value();
      else
        stream << part;

      if (part != *std::prev(path_parts.end())) stream << '/';
    }
  } else {
    if (path_.back() == '/')
      stream << path_.substr(0, path_.size() - 1);
    else
      stream << routine::utils::format_path(path_);
  }

  if (query_params_.size() > 0) {
    stream << '?';
    size_t i = 0;
    for (auto& param : query_params_) {
      stream << param.first << '=' << param.second.value();
      if (++i < query_params_.size()) stream << '&';
    }
  }

  stream << " HTTP/1.1\r\n";

  for (auto& header : headers_)
    stream << header.as_string() << "\r\n";

  if (!headers_.contains(http::Header::User_Agent)) stream << "User-Agent: Asio/RoutineHttp\r\n";
  if (body_) stream << "Content-Length: " << body_->size() << "\r\n";

  stream << "\r\n";
  if (body_) stream << body_->as_string();

  return stream.str();
}
