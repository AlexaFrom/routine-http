#pragma once

#include "http/body_storage.hpp"
#include "http/request.hpp"
#include "http/response.hpp"
#include "http/types.hpp"
#include <memory>

namespace routine::http {

  class RequestHandler {
  public:
    // DONT FORGET TO SPECIFY THE PATH OF RESOURCE HANDLER
    // inline static const std::string path = "/path/to/resource";

    // Executed in IO-bound threads.
    // > Return nullptr - to add to the queue,
    // > or return a ready Response_ptr to skip the queue and return to the client.
    virtual Response_ptr prepare_request(Request_ptr request) {
      if (request->method() != Method::Head)
        request->body() = std::make_unique<routine::http::MemoryBody>();
      return nullptr;
    }

    // Executed in threads bound to the processor.
    // > Return nullptr - to add to the queue again,
    // > or return a ready Response_ptr for sending to the client.
    virtual Response_ptr process_request(Request_ptr request) = 0;

    virtual ~RequestHandler() = default;
  };

  using RequestHandler_ptr = std::shared_ptr<routine::http::RequestHandler>;

}; // namespace routine::http
