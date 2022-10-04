cmake_minimum_required(VERSION 3.13)

#
# Project details
#

project("databento" VERSION 0.1.0 LANGUAGES CXX)

#
# Set project options
#

include(cmake/StandardSettings.cmake)
include(cmake/Utils.cmake)
message(STATUS "Started CMake for ${PROJECT_NAME} v${PROJECT_VERSION}...\n")

#
# Setup alternative names
#

if(${PROJECT_NAME}_USE_ALT_NAMES)
  string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWERCASE)
  string(TOUPPER ${PROJECT_NAME} PROJECT_NAME_UPPERCASE)
else()
  set(PROJECT_NAME_LOWERCASE ${PROJECT_NAME})
  set(PROJECT_NAME_UPPERCASE ${PROJECT_NAME})
endif()

if(NOT CMAKE_BUILD_TYPE)
  message(INFO "Defaulting to Debug build")
  set(CMAKE_BUILD_TYPE "Debug")
endif()

#
# Prevent building in the source directory
#

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there.\n")
endif()

#
# Add version header
#

configure_file(
  ${CMAKE_CURRENT_LIST_DIR}/cmake/version.hpp.in
  include/${PROJECT_NAME_LOWERCASE}/version.hpp
  @ONLY
)


#
# Find all headers and implementation files
#

include(cmake/SourcesAndHeaders.cmake)

if(${PROJECT_NAME}_BUILD_HEADERS_ONLY)
  if(${PROJECT_NAME}_VERBOSE_OUTPUT)
    verbose_message("Found the following headers:")
    foreach(header IN LIST headers)
      verbose_message("* ${header}")
    endforeach()
  endif()

  add_library(${PROJECT_NAME} INTERFACE)
else()
  if(${PROJECT_NAME}_VERBOSE_OUTPUT)
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
    ${headers}
    ${sources}
  )
endif()

set_target_properties(
  ${PROJECT_NAME}
  PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE}"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE}"
)

message(STATUS "Added all header and implementation files.\n")

#
# Set the project standard and warnings
#

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_11)
include(cmake/CompilerWarnings.cmake)
set_project_warnings(${PROJECT_NAME})

verbose_message("Applied compiler warnings.\n")

#
# Model project dependencies
#

include(FetchContent)
# JSON
if(${PROJECT_NAME}_USE_EXTERNAL_JSON)
  find_package(nlohmann_json 3.11.2 REQUIRED)
else()
  FetchContent_Declare(
    json 
    URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  )
  FetchContent_MakeAvailable(json)
endif()
# cpp-httplib
FetchContent_Declare(
  httplib
  URL https://github.com/yhirose/cpp-httplib/archive/refs/tags/v0.11.2.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(httplib)
find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
  set(HTTPLIB_IS_USING_OPENSSL TRUE)
endif()
find_library(zstd zstd REQUIRED)
find_package(Threads REQUIRED)

target_link_libraries(
  ${PROJECT_NAME}
  PUBLIC
    httplib
    OpenSSL::Crypto
    OpenSSL::SSL
    Threads::Threads
    zstd
  PRIVATE
    nlohmann_json::nlohmann_json
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

# Allow usage of header files in the `src` directory, but only for utilities
if(${PROJECT_NAME}_BUILD_HEADERS_ONLY)
  target_include_directories(
    ${PROJECT_NAME}
    INTERFACE
      ${CMAKE_BINARY_DIR}/include   # for generated version.hpp
      $<INSTALL_INTERFACE:include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  )
else()
  target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
      ${CMAKE_BINARY_DIR}/include   # for generated version.hpp
      $<INSTALL_INTERFACE:include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/src
  )
endif()

message(STATUS "Finished setting up include directories.")

#
# Provide alias to library for
#

add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

verbose_message("Project is now aliased as ${PROJECT_NAME}::${PROJECT_NAME}.\n")

#
# Format the project using the `clang-format` target (i.e: cmake --build build --target clang-format)
#

add_clang_format_target()

#
# Install library for easy downstream inclusion
#

# include(GNUInstallDirs)
# install(
#   TARGETS
#   ${PROJECT_NAME}
#   EXPORT
#   ${PROJECT_NAME}Targets
#   LIBRARY DESTINATION
#   ${CMAKE_INSTALL_LIBDIR}
#   ARCHIVE DESTINATION
#   ${CMAKE_INSTALL_LIBDIR}
#   INCLUDES DESTINATION
#   include
#   PUBLIC_HEADER DESTINATION
#   include
# )

# install(
#   EXPORT
#   ${PROJECT_NAME}Targets
#   FILE
#   ${PROJECT_NAME}Targets.cmake
#   NAMESPACE
#   ${PROJECT_NAME}::
#   DESTINATION
#   ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
# )

install(
  FILES
  ${CMAKE_CURRENT_BINARY_DIR}/include/${PROJECT_NAME_LOWERCASE}/version.hpp
  DESTINATION
  include/${PROJECT_NAME_LOWERCASE}
)

#
# Install the `include` directory
#

install(
  DIRECTORY
  include/${PROJECT_NAME_LOWERCASE}
  DESTINATION
  include
)

verbose_message("Install targets successfully built. Install with `cmake --build <build_directory> --target install --config <build_config>`.")

#
# Quick `ConfigVersion.cmake` creation
#

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${PROJECT_NAME}ConfigVersion.cmake
  VERSION
  ${PROJECT_VERSION}
  COMPATIBILITY
  SameMajorVersion
)

configure_package_config_file(
  ${CMAKE_CURRENT_LIST_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
  INSTALL_DESTINATION
  ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

install(
  FILES
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
  DESTINATION
  ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

#
# Generate export header if specified
#

if(${PROJECT_NAME}_GENERATE_EXPORT_HEADER)
  include(GenerateExportHeader)
  generate_export_header(${PROJECT_NAME})
  install(
    FILES
    ${PROJECT_BINARY_DIR}/${PROJECT_NAME_LOWERCASE}_export.h
    DESTINATION
    include
  )

  message(STATUS "Generated the export header `${PROJECT_NAME_LOWERCASE}_export.h` and installed it.")
endif()

message(STATUS "Finished building requirements for installing the package.\n")

#
# Unit testing setup
#

if(${PROJECT_NAME}_ENABLE_UNIT_TESTING)
  enable_testing()
  message(STATUS "Build unit tests for the project.")
  add_subdirectory(test)
endif()

if(${PROJECT_NAME}_ENABLE_EXAMPLES)
  message(STATUS "Build examples for the project.")
  add_subdirectory(example)
endif()
