cmake_minimum_required(VERSION 3.14)
project(webserver)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_DEBUG_LIBS OFF)
set(Boost_USE_RELEASE_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

option(USE_DEBUG "Use debug mode" OFF)

if(USE_DEBUG)
  set(CMAKE_BUILD_TYPE Debug)
  add_compile_definitions(DEBUG)
endif()

find_package(Boost REQUIRED COMPONENTS system)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  link_directories(${Boost_LIBRARY_DIRS})
endif()

add_executable(webserver
  ${CMAKE_SOURCE_DIR}/main.cpp
  ${CMAKE_SOURCE_DIR}/data.cpp
  ${CMAKE_SOURCE_DIR}/data_view.cpp
  ${CMAKE_SOURCE_DIR}/utils.cpp
  ${CMAKE_SOURCE_DIR}/request_header.cpp
  ${CMAKE_SOURCE_DIR}/request.cpp
  ${CMAKE_SOURCE_DIR}/response_header.cpp
  ${CMAKE_SOURCE_DIR}/response.cpp
  ${CMAKE_SOURCE_DIR}/static_server.cpp
)
target_link_libraries(webserver Boost::system)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest
  GIT_TAG        v1.14.0
)

FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(webserver_test
  ${CMAKE_SOURCE_DIR}/test/queue_test.cpp
  ${CMAKE_SOURCE_DIR}/data.cpp
  ${CMAKE_SOURCE_DIR}/test/data_test.cpp
  ${CMAKE_SOURCE_DIR}/data_view.cpp
  ${CMAKE_SOURCE_DIR}/test/data_view_test.cpp
  ${CMAKE_SOURCE_DIR}/test/data_buffer_test.cpp
  ${CMAKE_SOURCE_DIR}/utils.cpp
  ${CMAKE_SOURCE_DIR}/test/string_operation_test.cpp
  ${CMAKE_SOURCE_DIR}/request_header.cpp
  ${CMAKE_SOURCE_DIR}/test/request_header_test.cpp
  ${CMAKE_SOURCE_DIR}/request.cpp
  ${CMAKE_SOURCE_DIR}/test/request_test.cpp
  ${CMAKE_SOURCE_DIR}/response_header.cpp
  ${CMAKE_SOURCE_DIR}/test/response_header_test.cpp
  ${CMAKE_SOURCE_DIR}/response.cpp
  ${CMAKE_SOURCE_DIR}/test/response_test.cpp
  ${CMAKE_SOURCE_DIR}/test/mock_socket.cpp
  ${CMAKE_SOURCE_DIR}/test/connection_test.cpp
  ${CMAKE_SOURCE_DIR}/test/connection_pool_test.cpp
  ${CMAKE_SOURCE_DIR}/test/logger_test.cpp
)
target_link_libraries(webserver_test GTest::gtest_main Boost::system)

include(GoogleTest)
gtest_discover_tests(webserver_test)
