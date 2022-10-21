cmake_minimum_required(VERSION 3.13)

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
)

add_executable(metadata metadata.cpp)
target_link_libraries(
  metadata
  PRIVATE
    databento::databento
)

add_executable(readme readme.cpp)
target_link_libraries(
  readme
  PRIVATE
    databento::databento
)

add_executable(symbology-resolve symbology_resolve.cpp)
target_link_libraries(
  symbology-resolve
  PRIVATE
    databento::databento
)

add_executable(timeseries-stream timeseries_stream.cpp)
target_link_libraries(
  timeseries-stream
  PRIVATE
    databento::databento
)

add_executable(timeseries-stream-to-file timeseries_stream_to_file.cpp)
target_link_libraries(
  timeseries-stream-to-file
  PRIVATE
    databento::databento
)

#
# Set the compiler standard
#

target_compile_features(batch PUBLIC cxx_std_11)
target_compile_features(metadata PUBLIC cxx_std_11)
target_compile_features(readme PUBLIC cxx_std_11)
target_compile_features(symbology-resolve PUBLIC cxx_std_11)
target_compile_features(timeseries-stream PUBLIC cxx_std_11)
target_compile_features(timeseries-stream-to-file PUBLIC cxx_std_11)
