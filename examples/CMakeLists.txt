cmake_minimum_required(VERSION 3.24)

project(
  ${CMAKE_PROJECT_NAME}Examples
  LANGUAGES CXX
)
add_subdirectory(historical)
add_subdirectory(live)
