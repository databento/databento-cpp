cmake_minimum_required(VERSION 3.24)

project(DummyImport LANGUAGES CXX)

find_package(databento REQUIRED)

add_executable(dummy main.cpp)
target_link_libraries(dummy databento::databento)
