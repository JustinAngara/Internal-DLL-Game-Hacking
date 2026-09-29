cmake_minimum_required(VERSION 3.20)
project(ProcDriver C CXX)

string(REPLACE "/RTC1" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")

set(WDK_WINVER "0x0A00" CACHE STRING "")
set(WDK_NTDDI_VERSION "0x0A00000B" CACHE STRING "")
set(KMDF_VERSION "1.31" CACHE STRING "")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(WDK REQUIRED)

string(REPLACE "." ";" KMDF_PARTS "${KMDF_VERSION}")
list(GET KMDF_PARTS 0 KMDF_MAJOR)
list(GET KMDF_PARTS 1 KMDF_MINOR)

wdk_add_driver(ProcDriver
    KMDF ${KMDF_VERSION}
    src/driver.c
)
target_include_directories(ProcDriver PRIVATE include)
target_compile_options(ProcDriver PRIVATE /W4 /WX)
target_compile_definitions(ProcDriver PRIVATE
    KMDF_VERSION_MAJOR=${KMDF_MAJOR}
    KMDF_VERSION_MINOR=${KMDF_MINOR}
)

add_executable(ProcDriverTest test/test.c)
target_include_directories(ProcDriverTest PRIVATE include)
target_compile_definitions(ProcDriverTest PRIVATE UNICODE _UNICODE)
target_compile_options(ProcDriverTest PRIVATE /W4)

set(SIGN_CERT_NAME "" CACHE STRING "")
if(SIGN_CERT_NAME)
    add_custom_command(TARGET ProcDriver POST_BUILD
        COMMAND signtool sign /fd SHA256 /s PrivateCertStore /n "${SIGN_CERT_NAME}" "$<TARGET_FILE:ProcDriver>"
    )
endif()
