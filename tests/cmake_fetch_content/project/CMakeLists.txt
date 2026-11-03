cmake_minimum_required(VERSION 3.24)

project(DummyImport LANGUAGES CXX)

include(FetchContent)

FetchContent_Declare(databento SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
FetchContent_MakeAvailable(databento)

add_executable(with_namespace_target main.cpp)
target_link_libraries(with_namespace_target databento::databento)

add_executable(without_namespace_target main.cpp)
target_link_libraries(without_namespace_target databento)
