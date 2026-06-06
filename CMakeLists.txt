cmake_minimum_required(VERSION 3.20)
project(ProcessHooker)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

add_library(ProcessHooker SHARED ${SOURCES} "src/core/Main.h")

target_include_directories(ProcessHooker PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

message(STATUS "Sources found: ${SOURCES}")

target_compile_options(ProcessHooker PRIVATE /Y-)

target_compile_definitions(ProcessHooker PRIVATE
    WIN32_LEAN_AND_MEAN
    UNICODE
    _UNICODE
)
