cmake_minimum_required(VERSION 3.16)
project(pipe LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB ALL_SOURCES "src/*.cpp")

message(STATUS "Files found: ${ALL_SOURCES}")

if(NOT ALL_SOURCES)
    message(FATAL_ERROR "No source files found in src/ folder!")
endif()

add_executable(pipe ${ALL_SOURCES})
target_include_directories(pipe PRIVATE include)