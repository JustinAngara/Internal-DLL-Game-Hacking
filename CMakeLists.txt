cmake_minimum_required(VERSION 3.20)
project(ProcessHooker LANGUAGES C CXX ASM_MASM)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
             PROPERTY VS_STARTUP_PROJECT Loader)

#Process Hooker
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/CheatCodeBase/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/CheatCodeBase/src/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/CheatCodeBase/src/*.asm"
    "${CMAKE_CURRENT_SOURCE_DIR}/CheatCodeBase/CounterStrike/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/CheatCodeBase/CounterStrike/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/CheatCodeBase/CounterStrike/*.asm"
)
message(STATUS "DLL sources found: ${SOURCES}")

file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/CheatCodeBase/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/CheatCodeBase/src/*.inl"
    "${CMAKE_CURRENT_SOURCE_DIR}/CheatCodeBase/CounterStrike/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/CheatCodeBase/CounterStrike/*.inl"
)

add_library(ProcessHooker SHARED ${SOURCES} ${HEADERS})
target_compile_features(ProcessHooker PRIVATE cxx_std_17)
target_include_directories(ProcessHooker PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/CheatCodeBase/src"
)
target_compile_options(ProcessHooker PRIVATE 
    $<$<COMPILE_LANGUAGE:C,CXX>:/Y->
)
target_compile_definitions(ProcessHooker PRIVATE
    WIN32_LEAN_AND_MEAN
    UNICODE
    _UNICODE
)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    target_link_libraries(ProcessHooker PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/CheatCodeBase/src/ext/reserved_windows/ntdll/ntdll_x64.lib
    )
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    target_link_libraries(ProcessHooker PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/CheatCodeBase/src/ext/reserved_windows/ntdll/ntdll_x86.lib
    )
endif()

#LOADER EXE
file(GLOB_RECURSE LOADER_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Loader/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Loader/*.c"
)
message(STATUS "Loader sources found: ${LOADER_SOURCES}")

add_executable(Loader
    ${LOADER_SOURCES}
    "CheatCodeBase/src/ext/reserved_windows/ntdll/NtDllHandler.cpp"
)
target_compile_features(Loader PRIVATE cxx_std_17)
target_include_directories(Loader PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/Loader"
    "${CMAKE_CURRENT_SOURCE_DIR}/CheatCodeBase/src"
)
target_compile_definitions(Loader PRIVATE
    WIN32_LEAN_AND_MEAN
    UNICODE
    _UNICODE
)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    target_link_libraries(Loader PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/CheatCodeBase/src/ext/reserved_windows/ntdll/ntdll_x64.lib
    )
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    target_link_libraries(Loader PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/CheatCodeBase/src/ext/reserved_windows/ntdll/ntdll_x86.lib
    )
endif()

add_dependencies(Loader ProcessHooker)

# ANTI CHEAT TESTER
file(GLOB_RECURSE TESTER_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/AntiCheatTester/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/AntiCheatTester/*.c"
)
message(STATUS "Tester sources found: ${TESTER_SOURCES}")

add_executable(AntiCheatTester
    ${TESTER_SOURCES}
    "CheatCodeBase/src/ext/reserved_windows/ntdll/NtDllHandler.cpp"
)
target_compile_features(AntiCheatTester PRIVATE cxx_std_17)
target_include_directories(AntiCheatTester PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/AntiCheatTester"
    "${CMAKE_CURRENT_SOURCE_DIR}/CheatCodeBase/src"
)
target_compile_definitions(AntiCheatTester PRIVATE
    WIN32_LEAN_AND_MEAN
    UNICODE
    _UNICODE
)

target_compile_options(AntiCheatTester PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/Od /Ob0 /Zi>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-O0 -g>
)
target_link_options(AntiCheatTester PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/DEBUG>
)

target_link_options(AntiCheatTester PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/DEBUG /DYNAMICBASE:NO>
)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    target_link_libraries(AntiCheatTester PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/CheatCodeBase/src/ext/reserved_windows/ntdll/ntdll_x64.lib
    )
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    target_link_libraries(AntiCheatTester PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/CheatCodeBase/src/ext/reserved_windows/ntdll/ntdll_x86.lib
    )
endif()

add_dependencies(AntiCheatTester ProcessHooker)