cmake_minimum_required(VERSION 3.14)

project(DummyImport LANGUAGES CXX)

include(FetchContent)

get_filename_component(GIT_REPOSITORY_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../../../.. ABSOLUTE)
set(DATABENTO_ENABLE_UNIT_TESTING OFF CACHE INTERNAL "")
FetchContent_Declare(
  databento
  GIT_REPOSITORY ${GIT_REPOSITORY_DIRECTORY}
  GIT_TAG HEAD
  SOURCE_SUBDIR public/databento-cpp
)
FetchContent_MakeAvailable(databento)

add_executable(with_namespace_target main.cpp)
target_link_libraries(with_namespace_target databento::databento)

add_executable(without_namespace_target main.cpp)
target_link_libraries(without_namespace_target databento)
