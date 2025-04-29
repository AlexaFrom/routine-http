
# Routine Http

**Library in development!** The purpose of library is to relieve the developer of routine tasks, while developing simple RestFull api.

Support C++17 and newer, but in future maybe support C++20.
If you want use C++17 check [this](#building).

# Dependencies

 - Asio or Boost.Asio *(use flag -DUSE_BOOST_ASIO=ON)*
 - nlohmann/json
 - grpc & protobuf
 - OpenSSL
 - spdlog

> deps can change in the future

# Building
by default using clang C++23, before build please check CMakeLists.txt.
If you need to compile it on standard less than C++23, edit CMakeLists.txt to break spdlog dependencie from C++23 std::format and use fmt library.

Clang C++23 with default CMakeLists.txt building instruction:
```
mkdir build && cd build
cmake .. -DUSE_BOOST_ASIO=ON
cmake --build . --target routine
```
> use flag *USE_BOOST_ASIO* to resolve boost:: namespaces for asio library

## Example
```c++
#include "net/acceptor.hpp"
#include "scheduler.hpp"
#include <memory>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

// create class and override the RequestHandler methods
class DynamicEchoHandler : public routine::http::RequestHandler {
public:
  // DONT FORGET TO SPECIFY THE PATH OF RESOURCE HANDLER
  inline static const std::string path{"/api/echo/{arg}/hello"};

	// Executed in IO-bound threads.
	// > Return nullptr - to add to the queue,
	// > or return a ready Response_ptr to skip the queue and return to the client.
	routine::http::Response_ptr prepare_request(routine::http::Request_ptr request) override {
    if (request->method() != routine::http::Method::Head)
	    // make body if this implies a query
      // request->body() = std::make_unique<routine::http::JsonBody>();
      request->body() = std::make_unique<routine::http::MemoryBody>();
    return nullptr;
  }

  // Executed in threads bound to the processor.
  // > Return nullptr - to add to the queue again,
  // > or return a ready Response_ptr for sending to the client.
  routine::http::Response_ptr process_request(routine::http::Request_ptr request) override {
    auto response = std::make_unique<routine::http::Response>(
        routine::http::Status::OK, routine::http::Headers{}, std::move(request->body()));
    return response;
  }
};

int main() {
  // Initialize loggers
  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  console_sink->set_level(spdlog::level::trace);
  std::vector<spdlog::sink_ptr> sinks{console_sink};

  auto logger = routine::utils::make_logger("System", spdlog::level::trace, sinks);
  routine::utils::make_logger("Scheduler", spdlog::level::trace, sinks);
  routine::utils::make_logger("Acceptor", spdlog::level::trace, sinks);
  routine::utils::make_logger("Http", spdlog::level::trace, sinks);
  routine::utils::make_logger("Router", spdlog::level::trace, sinks);
  routine::utils::make_logger("ThreadPool", spdlog::level::trace, sinks);

  // Routing handlers
  auto router = std::make_unique<routine::http::RouteHandler>();
  // add your handlers
  router->add_handler<DynamicEchoHandler>();

  // Init main components
  auto scheduler = std::make_shared<routine::Scheduler>();
  scheduler->set_router(std::move(router));
  auto acceptor = std::make_unique<routine::net::Acceptor<routine::net::HttpSession>>(scheduler, 80);

  // Start
  acceptor->async_accept();
	// run 1 IO thread and 2 CPU thread workers
  scheduler->run(1, 2);
  // lock main thread
  scheduler->join_threads();
  return 0;
}
```

## Docs
All documents you can see in readme.md or 'docs/' folder.

## In development

 - Body types
	 - JsonBody
	 - FileBody
 -	TLS support for HTTPs
 -	GZIP, DEFLATE and BR support
 -	Chunked-Body
 -	Service controller
	 -	monitoring, stats and logs
	 -	*admin web ui
 -	Middleware components for Requests/Responses
 -	support others microservices with gRPC
 -	Handlers runtime benchmarks
 -	anything, but not unit tests ☺

>*The code may contain comments in russian lang. Using the interface you not notice it, but I apologize☺*

>**If something doesn't work, it's probably my fail. If it's not too much trouble, please let me know**

### License
Library is distributed under the MIT license. Any use or modification is not deny.
