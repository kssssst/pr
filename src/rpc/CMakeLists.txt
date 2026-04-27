cmake_minimum_required(VERSION 3.16)

# Генерируем MIDL файлы (если нужны)
find_program(MIDL_COMPILER midl.exe)

if(MIDL_COMPILER)
  add_custom_command(
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/TrayService.h
      ${CMAKE_CURRENT_BINARY_DIR}/TrayService_c.c
      ${CMAKE_CURRENT_BINARY_DIR}/TrayService_s.c
    COMMAND ${MIDL_COMPILER} 
      /out ${CMAKE_CURRENT_BINARY_DIR}
      /h TrayService.h
      /cstub TrayService_c.c
      /sstub TrayService_s.c
      ${CMAKE_CURRENT_SOURCE_DIR}/TrayService.idl
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/TrayService.idl
    COMMENT "Generating MIDL files..."
  )
  
  add_library(rpc_lib
    ${CMAKE_CURRENT_BINARY_DIR}/TrayService_c.c
  )
  target_include_directories(rpc_lib PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
  
else()
  message(STATUS "MIDL compiler not found, skipping RPC generation")
  add_library(rpc_lib INTERFACE)
endif()
