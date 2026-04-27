cmake_minimum_required(VERSION 3.16)

find_program(MIDL_COMPILER midl.exe)

if(NOT MIDL_COMPILER AND WIN32)
    file(GLOB MIDL_CANDIDATES
        "$ENV{WindowsSdkDir}/bin/$ENV{WindowsSDKVersion}/x64/midl.exe"
        "$ENV{WindowsSdkDir}/bin/$ENV{WindowsSDKVersion}/arm64/midl.exe"
        "C:/Program Files (x86)/Windows Kits/10/bin/*/x64/midl.exe"
        "C:/Program Files (x86)/Windows Kits/10/bin/*/arm64/midl.exe"
    )
    if(MIDL_CANDIDATES)
        list(SORT MIDL_CANDIDATES)
        list(GET MIDL_CANDIDATES -1 MIDL_COMPILER)
    endif()
endif()

if(NOT MIDL_COMPILER)
    message(FATAL_ERROR "midl.exe was not found. Build this project with the Windows SDK installed.")
endif()

set(RPC_GENERATED_DIR "${CMAKE_BINARY_DIR}/generated/rpc")
set(RPC_IDL "${CMAKE_CURRENT_SOURCE_DIR}/TrayServiceRpc.idl")
set(RPC_HEADER "${RPC_GENERATED_DIR}/TrayServiceRpc.h")
set(RPC_CLIENT "${RPC_GENERATED_DIR}/TrayServiceRpc_c.c")
set(RPC_SERVER "${RPC_GENERATED_DIR}/TrayServiceRpc_s.c")

add_custom_command(
    OUTPUT
        "${RPC_HEADER}"
        "${RPC_CLIENT}"
        "${RPC_SERVER}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${RPC_GENERATED_DIR}"
    COMMAND "${MIDL_COMPILER}"
        /nologo
        /app_config
        /out "${RPC_GENERATED_DIR}"
        /h "TrayServiceRpc.h"
        /client "stub"
        /server "stub"
        /cstub "TrayServiceRpc_c.c"
        /sstub "TrayServiceRpc_s.c"
        "${RPC_IDL}"
    DEPENDS "${RPC_IDL}"
    COMMENT "Generating Windows RPC stubs"
    VERBATIM
)

add_library(TrayRpcClient OBJECT "${RPC_CLIENT}")
target_include_directories(TrayRpcClient PUBLIC "${RPC_GENERATED_DIR}")
set_target_properties(TrayRpcClient PROPERTIES
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)

add_library(TrayRpcServer OBJECT "${RPC_SERVER}")
target_include_directories(TrayRpcServer PUBLIC "${RPC_GENERATED_DIR}")
set_target_properties(TrayRpcServer PROPERTIES
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)
