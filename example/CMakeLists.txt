cmake_minimum_required(VERSION 3.15)

#
# Project details
#

project(
  ${CMAKE_PROJECT_NAME}Examples
  LANGUAGES CXX
)

add_executable(metadata metadata.cpp)
target_link_libraries(
  metadata
  PRIVATE
    databento::databento
    nlohmann_json::nlohmann_json
)
target_include_directories(
  metadata
  PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

#
# Set the compiler standard
#

target_compile_features(metadata PUBLIC cxx_std_11)

