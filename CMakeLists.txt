cmake_minimum_required(VERSION 3.20)
project(ProcessHooker)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)
message(STATUS "Sources found: ${SOURCES}")   

file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.inl"
)

add_library(ProcessHooker SHARED ${SOURCES} ${HEADERS} "src/HackedGames/CounterStrike/Recoil.cpp" "src/HackedGames/CounterStrike/Recoil.h" "src/Obfuscation/Polymorphism/Polymorphic.h" "src/Obfuscation/Polymorphism/Polymorphic.cpp" "src/Obfuscation/Registery/ObfuscateMethods.cpp" "src/Obfuscation/HideProcess/HideProcess.h" "src/Obfuscation/HideProcess/HideProcess.cpp" "src/gui/Gui.h" "src/gui/Category.h" "src/gui/Feature.h" "src/ext/minhook/hde/hde64.c" "src/ext/minhook/hde/hde32.c" "src/Gui/Gui.cpp")

# add_library(ProcessHooker SHARED ${SOURCES} ${HEADERS} "src/sdk/Math/Math.h" "src/sdk/Math/Math.inl" "src/Game/HackedGames/CounterStrike/HackManager.h")

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
