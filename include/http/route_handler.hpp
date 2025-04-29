#pragma once

#include "request_handler.hpp"
#include "utils/loggable_object.hpp"
#include "utils/utils.hpp"
#include <functional>
#include <memory>
#include <type_traits>

namespace routine::http {

  class RouteHandler : routine::utils::LoggableObject {
    using Handler_creator = std::function<std::shared_ptr<routine::http::RequestHandler>()>;
    using Handler_creator_ptr = std::unique_ptr<Handler_creator>;

    struct ResourceHandler {
      Handler_creator_ptr resource;
      std::unordered_map<std::string, ResourceHandler> nodes;
    };

  public:
    RouteHandler() : LoggableObject("Router") {}

#if defined(__cplusplus) && __cplusplus >= 202002L
    // C++20 and newer
    template <typename T>
      requires std::is_base_of_v<RequestHandler, T>
#else
    // C++17 and lower
    template <typename T, typename = std::enable_if_t<std::is_base_of_v<RequestHandler, T>>>
#endif
    void add_handler() {
      static_assert(
          std::is_same_v<decltype(T::path), const std::string>,
          "\n\n\tYou need to define 'path' field in the 'RouteHandler::add_handler<T>()' handler."
          "\n\tType 'T' must contain T::path with 'const std::string' type."
          "\n\tFor more details, see '/docs/http/RequestHandler'\n");

      auto lambda_handler_creator =
          std::make_unique<Handler_creator>([]() { return std::make_shared<T>(); });

      if (T::path.find('{') == std::string::npos) {
        // Static Handlers
        info("Successfully added a static handler for '{}'", T::path);
        static_handlers_[routine::utils::format_path(T::path)] = std::move(lambda_handler_creator);
      } else {
        // Dynamic Handlers
        ResourceHandler* node = &dynamic_handlers_;
        std::string current_full_path{'/'};
        std::string path{T::path};
        auto path_parts = routine::utils::splitString<std::string>(path, '/', 4);

        for (const auto& key : path_parts) {
          // check if key is a path parameter
          if (key[0] == '{') {
            auto it = std::find_if(node->nodes.begin(), node->nodes.end(),
                                   [](const std::pair<const std::string, ResourceHandler>& pair) {
                                     return pair.first[0] == '{';
                                   });

            // if current level already have parameter AND it is not equal to the current one
            if (it != node->nodes.end() && it->first != key)
              throw std::invalid_argument(
                  std::format("The path '{}' already has a another parameter '{}' on it.",
                              current_full_path + key, it->first));
          }

          node = &node->nodes[key];
          current_full_path.append(key);
        }

        info("Successfully added a dynamic handler for '{}'", T::path);
        node->resource = std::move(lambda_handler_creator);
      }
    }

    std::shared_ptr<routine::http::RequestHandler> route(Request& request) {
      // check if ResourceHandler have in static_handlers_
      if (auto it = static_handlers_.find(request.path()); it != static_handlers_.end())
        return (*it->second)();

      // otherwise search in dynamic_handlers_
      ResourceHandler* node = &dynamic_handlers_;
      auto path_parts = routine::utils::splitString<std::string>(request.path(), '/', 4);

      for (const auto& key : path_parts) {
        auto it = node->nodes.find(key);
        if (it == node->nodes.end()) {
          // if key not found, look for the presence of the parameter at this level
          it = std::find_if(node->nodes.begin(), node->nodes.end(),
                            [](const std::pair<const std::string&, ResourceHandler&> pair) {
                              return pair.first[0] == '{';
                            });
          if (it == node->nodes.end())
            return nullptr;
          else
            request.path_params().emplace(it->first.substr(1, it->first.size() - 2), key);
        }
        node = &it->second;
      }

      return node->resource ? (*node->resource)() : nullptr;
    }

  private:
    ResourceHandler dynamic_handlers_;
    std::unordered_map<std::string, Handler_creator_ptr> static_handlers_;
  };

} // namespace routine::http
