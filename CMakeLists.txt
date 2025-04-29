cmake_minimum_required(VERSION 4.0)
project(routine)

# Build options
option(USE_BOOST_ASIO "Use Boost.Asio instead of standalone Asio library" ON)

# Flags
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_C_COMPILER clang)

add_definitions(-DSPDLOG_USE_STD_FORMAT=ON)
add_definitions(-DUSE_BOOST_ASIO=ON)

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg") set(CMAKE_EXE_LINKER_FLAGS
# "${CMAKE_EXE_LINKER_FLAGS} -pg")

include_directories(${PROJECT_SOURCE_DIR}/include)

add_library(
  routine SHARED
  source/scheduler.cpp
  source/net/http_session.cpp
  source/http/headers.cpp
  source/http/request.cpp
  source/http/body_storage.cpp
  source/thread_pool.cpp
  source/http/response.cpp)

target_include_directories(routine PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_subdirectory(examples)
# add_subdirectory(tests)
