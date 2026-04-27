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
set(RPC_ACF "${CMAKE_CURRENT_SOURCE_DIR}/TrayServiceRpc.acf")
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
        /out "${RPC_GENERATED_DIR}"
        /h "TrayServiceRpc.h"
        /acf "${RPC_ACF}"
        /client "stub"
        /server "stub"
        /cstub "TrayServiceRpc_c.c"
        /sstub "TrayServiceRpc_s.c"
        "${RPC_IDL}"
    DEPENDS "${RPC_IDL}" "${RPC_ACF}"
    COMMENT "Generating Windows RPC stubs"
    VERBATIM
)

add_custom_target(TrayRpcGenerated
    DEPENDS
        "${RPC_HEADER}"
        "${RPC_CLIENT}"
        "${RPC_SERVER}"
)

set(TRAY_RPC_GENERATED_DIR "${RPC_GENERATED_DIR}" PARENT_SCOPE)
set(TRAY_RPC_CLIENT_SOURCE "${RPC_CLIENT}" PARENT_SCOPE)
set(TRAY_RPC_SERVER_SOURCE "${RPC_SERVER}" PARENT_SCOPE)
