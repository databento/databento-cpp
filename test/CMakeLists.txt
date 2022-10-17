cmake_minimum_required(VERSION 3.13)

#
# Project details
#

project(
  ${CMAKE_PROJECT_NAME}Tests
  LANGUAGES CXX
)

verbose_message("Adding tests under ${CMAKE_PROJECT_NAME}Tests...")

#
# Set the sources for the unit tests and add the executable(s)
#

set(
  test_headers
  include/mock/mock_server.hpp
)

set(
  test_sources
  src/datetime_tests.cpp
  src/dbz_parser_tests.cpp
  src/historical_tests.cpp
  src/http_client_tests.cpp
  src/mock_server.cpp
  src/parse_stream_tests.cpp
)
add_executable(${PROJECT_NAME} ${test_headers} ${test_sources})
find_package(Threads REQUIRED)

target_link_libraries(
  ${PROJECT_NAME}
  PUBLIC
    ${CMAKE_THREAD_LIBS_INIT}
  PRIVATE
    nlohmann_json::nlohmann_json
)

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE
    ${CMAKE_SOURCE_DIR}/src  # allow testing of private classes
)

#
# Set the compiler standard
#

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_11)
# Ensure std::string debug info is included
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_options(${PROJECT_NAME} PRIVATE -fstandalone-debug)
endif()

#
# Setup code coverage if enabled
#

if (${CMAKE_PROJECT_NAME}_ENABLE_CODE_COVERAGE)
  target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC -O0 -g -fprofile-arcs -ftest-coverage)
  target_link_options(${CMAKE_PROJECT_NAME} PUBLIC -fprofile-arcs -ftest-coverage)
  verbose_message("Code coverage is enabled and provided with GCC.")
endif()

#
# Load gtest
#

if(${CMAKE_PROJECT_NAME}_BUILD_EXECUTABLE)
  set(${CMAKE_PROJECT_NAME}_TEST_LIB ${CMAKE_PROJECT_NAME}_LIB)
else()
  set(${CMAKE_PROJECT_NAME}_TEST_LIB ${CMAKE_PROJECT_NAME})
endif()

find_package(GTest REQUIRED)

if(${CMAKE_PROJECT_NAME}_USE_GOOGLE_MOCK)
  set(GOOGLE_MOCK_LIBRARIES GTest::gmock GTest::gmock_main)
endif()

target_link_libraries(
  ${PROJECT_NAME} 
  PUBLIC
    GTest::GTest
    GTest::Main
    ${GOOGLE_MOCK_LIBRARIES}
    ${${CMAKE_PROJECT_NAME}_TEST_LIB}
)

#
# Add the unit tests
#

add_test(
  NAME
    ${PROJECT_NAME}
  COMMAND
    ${PROJECT_NAME}
)

#
# Copy test data
#

file(COPY data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
add_compile_definitions(TEST_BUILD_DIR="${CMAKE_CURRENT_BINARY_DIR}")

verbose_message("Finished adding unit tests for ${CMAKE_PROJECT_NAME}.")
