cmake_minimum_required(VERSION 3.15)

#
# Project details
#

project(
  ${CMAKE_PROJECT_NAME}Examples
  LANGUAGES CXX
)

add_executable(batch batch.cpp)
target_link_libraries(
  batch
  PRIVATE
    databento::databento
    nlohmann_json::nlohmann_json
)
target_include_directories(
  batch
  PRIVATE
    ${CMAKE_SOURCE_DIR}/include
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

add_executable(symbology-resolve symbology_resolve.cpp)
target_link_libraries(
  symbology-resolve
  PRIVATE
    databento::databento
    nlohmann_json::nlohmann_json
)
target_include_directories(
  symbology-resolve
  PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

#
# Set the compiler standard
#

target_compile_features(batch PUBLIC cxx_std_11)
target_compile_features(metadata PUBLIC cxx_std_11)
target_compile_features(symbology-resolve PUBLIC cxx_std_11)

