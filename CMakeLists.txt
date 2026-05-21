cmake_minimum_required(VERSION 3.20)
project(ProcessHooker)

file(GLOB_RECURSE SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

add_library(ProcessHooker SHARED ${SOURCES} "src/sdk/Memory/PatternScan.h" "src/sdk/Memory/PatternScan.cpp")

target_include_directories(ProcessHooker PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_compile_options(ProcessHooker PRIVATE /Y-)

target_compile_definitions(ProcessHooker PRIVATE
    WIN32_LEAN_AND_MEAN
)