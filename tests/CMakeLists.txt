cmake_minimum_required(VERSION 3.24)

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
  include/mock/mock_log_receiver.hpp
  include/mock/mock_lsg_server.hpp
  include/mock/mock_tcp_server.hpp
  include/temp_file.hpp
)

set(
  test_sources
  src/batch_tests.cpp
  src/buffer_tests.cpp
  src/datetime_tests.cpp
  src/dbn_decoder_tests.cpp
  src/dbn_encoder_tests.cpp
  src/dbn_file_store_tests.cpp
  src/dbn_tests.cpp
  src/file_stream_tests.cpp
  src/flag_set_tests.cpp
  src/historical_tests.cpp
  src/http_client_tests.cpp
  src/live_blocking_tests.cpp
  src/live_tests.cpp
  src/live_threaded_tests.cpp
  src/log_tests.cpp
  src/metadata_tests.cpp
  src/mock_http_server.cpp
  src/mock_lsg_server.cpp
  src/mock_tcp_server.cpp
  src/pretty_tests.cpp
  src/record_tests.cpp
  src/scoped_thread_tests.cpp
  src/stream_op_helper_tests.cpp
  src/symbol_map_tests.cpp
  src/symbology_tests.cpp
  src/tcp_client_tests.cpp
  src/zstd_stream_tests.cpp
)
add_executable(${PROJECT_NAME} ${test_headers} ${test_sources})
if(WIN32)
  # Disable warnings
  target_compile_options(${PROJECT_NAME} PRIVATE /w)
endif()
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

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
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
  target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC
      GTest::GTest
      GTest::Main
      ${${CMAKE_PROJECT_NAME}_TEST_LIB}
  )

else()
  include(FetchContent)
  if(WIN32)
    # Visual Studio by default links C runtimes dynamically but gtest by default links them statically
    set(gtest_force_shared_crt ON CACHE BOOL "Link dynamic CRT" FORCE)
  endif()
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/releases/download/v1.17.0/googletest-1.17.0.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  )
  FetchContent_MakeAvailable(googletest)
  target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC
      gmock_main
      ${${CMAKE_PROJECT_NAME}_TEST_LIB}
  )
  # Ignore compiler warnings in headers
  add_system_include_property(gtest)
endif()


#
# Add the unit tests
#

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})

#
# Copy test data
#

file(COPY data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
add_compile_definitions(TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")

verbose_message("Finished adding unit tests for ${CMAKE_PROJECT_NAME}.")

add_subdirectory(cmake_fetch_content)
add_subdirectory(cmake_import)
