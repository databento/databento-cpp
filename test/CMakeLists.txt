cmake_minimum_required(VERSION 3.14)

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
  include/mock/mock_http_server.hpp
  include/mock/mock_tcp_server.hpp
  include/temp_file.hpp
)

set(
  test_sources
  src/batch_tests.cpp
  src/datetime_tests.cpp
  src/dbz_tests.cpp
  src/dbz_parser_tests.cpp
  src/historical_tests.cpp
  src/live_tests.cpp
  src/metadata_tests.cpp
  src/mock_http_server.cpp
  src/mock_tcp_server.cpp
  src/record_tests.cpp
  src/scoped_thread_tests.cpp
  src/shared_channel_tests.cpp
  src/stream_op_helper_tests.cpp
  src/symbology_tests.cpp
  src/tcp_client_tests.cpp
)
add_executable(${PROJECT_NAME} ${test_headers} ${test_sources})
find_package(Threads REQUIRED)

target_link_libraries(
  ${PROJECT_NAME}
  PUBLIC
    Threads::Threads
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

if(${PROJECT_NAME_UPPERCASE}_USE_EXTERNAL_GTEST)
  find_package(GTest REQUIRED)
else()
  include(FetchContent)
  if(CMAKE_VERSION VERSION_LESS 3.24)
    FetchContent_Declare(
      googletest
      URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.tar.gz
    )
  else()
    # DOWNLOAD_EXTRACT_TIMESTAMP added in 3.24
    FetchContent_Declare(
      googletest
      URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.tar.gz
      DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
  endif()
  FetchContent_MakeAvailable(googletest)
endif()

target_link_libraries(
  ${PROJECT_NAME}
  PUBLIC
    gtest
    gtest_main
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

add_subdirectory(cmake_fetch_content)
