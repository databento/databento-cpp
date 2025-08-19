cmake_minimum_required(VERSION 3.24..4.0)

#
# Project details
#

project(
  databento
  VERSION 0.42.0
  LANGUAGES CXX
  DESCRIPTION "Official Databento client library"
)
string(TOUPPER ${PROJECT_NAME} PROJECT_NAME_UPPERCASE)

#
# Main project check
#

set(IS_MAIN_PROJECT OFF)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(IS_MAIN_PROJECT ON)
endif()

#
# Set project options
#

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(cmake/StandardSettings.cmake)
include(cmake/StaticAnalyzers.cmake)
include(cmake/Utils.cmake)
message(STATUS "Started CMake for ${PROJECT_NAME} v${PROJECT_VERSION}...")

if(NOT CMAKE_BUILD_TYPE)
  if(IS_MAIN_PROJECT)
    message(STATUS "Defaulting to Debug build")
    set(CMAKE_BUILD_TYPE "Debug")
  else()
    message(STATUS "Defaulting to RelWithDebInfo build")
    set(CMAKE_BUILD_TYPE "RelWithDebInfo")
  endif()
endif()

#
# Prevent building in the source directory
#

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there.")
endif()

#
# Check endianness
#

include(TestBigEndian)
test_big_endian(IS_BIG_ENDIAN)
if(IS_BIG_ENDIAN)
  message(FATAL_ERROR "Big-endian platforms aren't supported because DBN parsing assumes the data is little-endian and in native byte order.")
endif()

#
# Add templated headers
#

configure_file(
  ${CMAKE_CURRENT_LIST_DIR}/cmake/version.hpp.in
  include/${PROJECT_NAME}/version.hpp
  @ONLY
)
configure_file(
  ${CMAKE_CURRENT_LIST_DIR}/cmake/system.hpp.in
  include/${PROJECT_NAME}/system.hpp
  @ONLY
)

#
# Find all headers and implementation files
#

include(cmake/SourcesAndHeaders.cmake)

if(${PROJECT_NAME_UPPERCASE}_VERBOSE_OUTPUT)
  verbose_message("Found the following sources:")
  foreach(source IN LISTS sources)
    verbose_message("* ${source}")
  endforeach()
  verbose_message("Found the following headers:")
  foreach(header IN LISTS headers)
    verbose_message("* ${header}")
  endforeach()
endif()

add_library(
  ${PROJECT_NAME}
  STATIC
  ${headers}
  ${sources}
)

set_target_properties(
  ${PROJECT_NAME}
  PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE}"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE}"
)

message(STATUS "Added all header and implementation files.")

#
# Set the project standard and warnings
#

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)
include(cmake/CompilerWarnings.cmake)
set_target_warnings(${PROJECT_NAME})
include(cmake/Sanitizers.cmake)

# Ensure std::string debug info is included
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_options(${PROJECT_NAME} PRIVATE -fstandalone-debug)
endif()

#
# Project dependencies
#

find_package(OpenSSL REQUIRED)
find_package(zstd REQUIRED)
if(NOT TARGET zstd::libzstd)
  if(TARGET zstd::libzstd_shared)
    add_library(zstd::libzstd ALIAS zstd::libzstd_shared)
  elseif(TARGET zstd::libzstd_static)
    add_library(zstd::libzstd ALIAS zstd::libzstd_static)
  endif()
endif()
find_package(Threads REQUIRED)

include(FetchContent)
# JSON
if(${PROJECT_NAME_UPPERCASE}_USE_EXTERNAL_JSON)
  # Check if json target already exists
  if(TARGET nlohmann_json::nlohmann_json)
    get_target_property(NLOHMANN_JSON_SOURCE_DIR nlohmann_json::nlohmann_json SOURCE_DIR)
    message(STATUS "nlohmann_json::nlohmann_json already available as a target at ${NLOHMANN_JSON_SOURCE_DIR}")
    get_target_property(NLOHMANN_JSON_INCLUDE_DIRS nlohmann_json::nlohmann_json INTERFACE_INCLUDE_DIRECTORIES)
    if(NLOHMANN_JSON_INCLUDE_DIRS)
      message(STATUS "nlohmann_json::nlohmann_json include directories: ${NLOHMANN_JSON_INCLUDE_DIRS}")
    endif()
  else()
    find_package(nlohmann_json REQUIRED)
  endif()
else()
  set(json_version 3.11.3)
  # Required to correctly install nlohmann_json
  set(JSON_Install ON)
  FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v${json_version}/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  )
  FetchContent_MakeAvailable(json)
  # Ignore compiler warnings in headers
  add_system_include_property(nlohmann_json)
endif()
# cpp-httplib
if(${PROJECT_NAME_UPPERCASE}_USE_EXTERNAL_HTTPLIB)
  # Check if httplib target already exists
  if(TARGET httplib::httplib)
    get_target_property(HTTPLIB_SOURCE_DIR httplib::httplib SOURCE_DIR)
    message(STATUS "httplib::httplib already available as a target at ${HTTPLIB_SOURCE_DIR}")
    get_target_property(HTTPLIB_INCLUDE_DIRS httplib::httplib INTERFACE_INCLUDE_DIRECTORIES)
    if(HTTPLIB_INCLUDE_DIRS)
      message(STATUS "httplib::httplib include directories: ${HTTPLIB_INCLUDE_DIRS}")
    endif()
  else()
    find_package(httplib REQUIRED)
  endif()
else()
  set(httplib_version 0.20.0)
  FetchContent_Declare(
    httplib
    URL https://github.com/yhirose/cpp-httplib/archive/refs/tags/v${httplib_version}.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  )
  FetchContent_MakeAvailable(httplib)
  # Ignore compiler warnings in headers
  add_system_include_property(httplib)
endif()
# date
if(${PROJECT_NAME_UPPERCASE}_USE_EXTERNAL_DATE)
  # Check if date target already exists
  if(TARGET date::date)
    get_target_property(DATE_SOURCE_DIR date::date SOURCE_DIR)
    message(STATUS "date::date already available as a target at ${DATE_SOURCE_DIR}")
    get_target_property(DATE_INCLUDE_DIRS date::date INTERFACE_INCLUDE_DIRECTORIES)
    if(DATE_INCLUDE_DIRS)
      message(STATUS "date::date include directories: ${DATE_INCLUDE_DIRS}")
    endif()
  else()
    find_package(date REQUIRED)
  endif()
else()
  set(date_version 3.0.3)
  FetchContent_Declare(
    date_src
    URL https://github.com/HowardHinnant/date/archive/refs/tags/v${date_version}.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  )
  FetchContent_MakeAvailable(date_src)
  # Ignore compiler warnings in headers
  add_system_include_property(date)
endif()

target_link_libraries(
  ${PROJECT_NAME}
  PUBLIC
    date::date
    httplib::httplib
    nlohmann_json::nlohmann_json
    OpenSSL::Crypto
    OpenSSL::SSL
    Threads::Threads
    zstd::libzstd
)

target_compile_definitions(
  ${PROJECT_NAME}
  PUBLIC
    CPPHTTPLIB_OPENSSL_SUPPORT
)

verbose_message("Successfully added all dependencies and linked against them.")

#
# Set the build/user include directories
#

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>   # for generated version.hpp and system.hpp
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

message(STATUS "Finished setting up include directories.")

#
# Provide alias to library for
#

add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

verbose_message("Project is now aliased as ${PROJECT_NAME}::${PROJECT_NAME}.")

#
# Format the project using the `clang-format` target (i.e: cmake --build build --target clang-format)
#

add_clang_format_target()

#
# Install library for easy downstream inclusion
#

set(targets ${PROJECT_NAME})
if(NOT ${PROJECT_NAME_UPPERCASE}_USE_EXTERNAL_JSON)
  list(APPEND targets nlohmann_json)
endif()
if(NOT ${PROJECT_NAME_UPPERCASE}_USE_EXTERNAL_HTTPLIB)
  list(APPEND targets httplib)
endif()

include(GNUInstallDirs)
install(
  TARGETS
    ${targets}
  EXPORT
    ${PROJECT_NAME}Targets
  LIBRARY DESTINATION
    ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION
    ${CMAKE_INSTALL_LIBDIR}
  INCLUDES DESTINATION
    ${CMAKE_INSTALL_INCLUDEDIR}
  PUBLIC_HEADER DESTINATION
    ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
  EXPORT
    ${PROJECT_NAME}Targets
  FILE
    ${PROJECT_NAME}Targets.cmake
  NAMESPACE
    ${PROJECT_NAME}::
  DESTINATION
    ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

#
# Install the `include` directory
#

set(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})
install(
  DIRECTORY
    include/${PROJECT_NAME}
  DESTINATION
    ${CMAKE_INSTALL_INCLUDEDIR}
)
install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/include/${PROJECT_NAME}/system.hpp
    ${CMAKE_CURRENT_BINARY_DIR}/include/${PROJECT_NAME}/version.hpp
  DESTINATION
    ${INCLUDE_INSTALL_DIR}
)

#
# Install license
#

install(
  FILES
    "LICENSE"
  DESTINATION
    ${CMAKE_INSTALL_DATADIR}/licenses/${PROJECT_NAME}
)

#
# `ConfigVersion.cmake` creation
#

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${PROJECT_NAME}ConfigVersion.cmake
  VERSION
    ${PROJECT_VERSION}
  COMPATIBILITY
    SameMinorVersion
)

#
# Generate package configuration and install it
#

configure_package_config_file(
  ${CMAKE_CURRENT_LIST_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
  PATH_VARS INCLUDE_INSTALL_DIR
)
install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    # Install so it can be used in databentoConfig.cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Findzstd.cmake
  DESTINATION
    ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

verbose_message("Install targets successfully built. Install with `cmake --build <build_directory> --target install --config <build_config>`.")

#
# Generate export header if specified
#

if(${PROJECT_NAME_UPPERCASE}_GENERATE_EXPORT_HEADER)
  include(GenerateExportHeader)
  generate_export_header(${PROJECT_NAME})
  install(
    FILES
      ${PROJECT_BINARY_DIR}/${PROJECT_NAME}_export.h
    DESTINATION
      ${CMAKE_INSTALL_INCLUDEDIR}
  )

  message(STATUS "Generated the export header `${PROJECT_NAME}_export.h` and installed it.")
endif()

message(STATUS "Finished building requirements for installing the package.")

#
# Platform-specific
#
if(WIN32)
    add_compile_definitions(NOMINMAX)
endif()

#
# Unit testing setup
#

if(${PROJECT_NAME_UPPERCASE}_ENABLE_UNIT_TESTING)
  unset(CMAKE_CXX_CPPCHECK) # disable cppcheck for tests
  unset(CMAKE_CXX_CLANG_TIDY) # disable clang-tidy for tests
  enable_testing()
  message(STATUS "Build unit tests for the project.")
  add_subdirectory(tests)
endif()

if(${PROJECT_NAME_UPPERCASE}_ENABLE_EXAMPLES)
  unset(CMAKE_CXX_CPPCHECK) # disable cppcheck for examples
  unset(CMAKE_CXX_CLANG_TIDY) # disable clang-tidy for examples
  message(STATUS "Build examples for the project.")
  add_subdirectory(examples)
endif()
