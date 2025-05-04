cmake_minimum_required(VERSION 3.14)
project(routine)

# Build options
option(USE_BOOST_ASIO "Use Boost.Asio instead of standalone Asio library" ON)
option(SPDLOG_FMT_EXTERNAL "Use external fmt instead of bundled" ON)

# Flags
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_C_COMPILER clang)

add_definitions(-DUSE_BOOST_ASIO=${USE_BOOST_ASIO})
add_definitions(-DSPDLOG_FMT_EXTERNAL=${SPDLOG_FMT_EXTERNAL})

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg") set(CMAKE_EXE_LINKER_FLAGS
# "${CMAKE_EXE_LINKER_FLAGS} -pg")

find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)

add_library(
  routine STATIC
  source/scheduler.cpp
  source/net/http_session.cpp
  source/http/headers.cpp
  source/http/request.cpp
  source/http/body_storage.cpp
  source/thread_pool.cpp
  source/http/response.cpp)

target_link_libraries(routine PRIVATE fmt::fmt spdlog::spdlog)

target_include_directories(routine PUBLIC ${PROJECT_SOURCE_DIR}/include)

if(USE_BOOST_ASIO)
  find_package(Boost REQUIRED COMPONENTS system)
  target_include_directories(routine PRIVATE ${Boost_INCLUDE_DIRS})
  target_link_libraries(routine PRIVATE Boost::system)
else()
  find_path(ASIO_INCLUDE_DIR asio.hpp)
  if(NOT ASIO_INCLUDE_DIR)
    message(FATAL_ERROR "ASIO_INCLUDE_DIR not found")
  endif()
  target_include_directories(routine PRIVATE ${ASIO_INCLUDE_DIR})
endif()

add_subdirectory(examples)
# add_subdirectory(tests)

install(
  TARGETS routine
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ DESTINATION include/routine)
