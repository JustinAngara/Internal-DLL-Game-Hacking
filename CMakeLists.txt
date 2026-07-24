cmake_minimum_required(VERSION 3.20)
project(ProcessHooker LANGUAGES C CXX ASM_MASM)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
             PROPERTY VS_STARTUP_PROJECT Loader)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.asm"
)
message(STATUS "DLL sources found: ${SOURCES}")

file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.inl"
)

add_library(ProcessHooker SHARED ${SOURCES} ${HEADERS})

target_compile_features(ProcessHooker PRIVATE cxx_std_17)

target_include_directories(ProcessHooker PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
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
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ext/reserved_windows/ntdll/ntdll_x64.lib
    )
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    target_link_libraries(ProcessHooker PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ext/reserved_windows/ntdll/ntdll_x86.lib
    )
endif()


file(GLOB_RECURSE LOADER_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Loader/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Loader/*.c"
)
message(STATUS "Loader sources found: ${LOADER_SOURCES}")

add_executable(Loader
    ${LOADER_SOURCES}
    "src/ext/reserved_windows/ntdll/NtDllHandler.cpp"
)

target_compile_features(Loader PRIVATE cxx_std_17)

target_include_directories(Loader PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/Loader"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_compile_definitions(Loader PRIVATE
    WIN32_LEAN_AND_MEAN
    UNICODE
    _UNICODE
)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    target_link_libraries(Loader PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ext/reserved_windows/ntdll/ntdll_x64.lib
    )
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    target_link_libraries(Loader PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ext/reserved_windows/ntdll/ntdll_x86.lib
    )
endif()

add_dependencies(Loader ProcessHooker)