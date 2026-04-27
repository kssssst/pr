cmake_minimum_required(VERSION 3.16)

project(TrayService)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(TrayService
    main.cpp
    $<TARGET_OBJECTS:TrayRpcServer>
)

target_compile_definitions(TrayService PRIVATE
    _UNICODE
    UNICODE
    WIN32_LEAN_AND_MEAN
)

if(MSVC)
    target_compile_options(TrayService PRIVATE /W4 /utf-8)
endif()

target_link_libraries(TrayService
    PRIVATE
        TrayRpcServer
        advapi32
        userenv
        wtsapi32
        rpcrt4
)

set_target_properties(TrayService PROPERTIES
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}/bin"
)
