#include "http/request.hpp"
#include "utils/utils.hpp"
#include <algorithm>
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
