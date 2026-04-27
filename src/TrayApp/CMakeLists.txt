cmake_minimum_required(VERSION 3.16)

project(TrayApp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Источники приложения
set(TRAYAPP_SOURCES
    main.cpp
    MainWindow.cpp
    MainWindow.h
    ServiceClient.cpp
    ServiceClient.h
    $<TARGET_OBJECTS:TrayRpcClient>
    pch.cpp
    pch.h
)

# Создаем исполняемый файл
add_executable(TrayApp WIN32 ${TRAYAPP_SOURCES})

# Подключаемые библиотеки
target_link_libraries(TrayApp
    PRIVATE
        shell32              # Shell API (иконки в трее)
        user32               # User API (окна, сообщения)
        gdi32                # Graphics Device Interface
        comctl32             # Common Controls
        advapi32             # Service Control Manager
        rpcrt4               # Windows RPC
)

# Параметры компиляции
if(MSVC)
    target_compile_options(TrayApp PRIVATE /W4 /utf-8)
    
    # Отключаем предупреждение о deprecated функциях
    target_compile_definitions(TrayApp PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# Параметры препроцессора
target_compile_definitions(TrayApp PRIVATE
    _UNICODE
    UNICODE
    WIN32_LEAN_AND_MEAN
)

# Включаемые пути
target_include_directories(TrayApp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/generated/rpc
)

add_dependencies(TrayApp TrayRpcClient)

# Выходной файл
set_target_properties(TrayApp PROPERTIES
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}/bin"
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Precompiled headers
if(MSVC)
    set_source_files_properties(
        pch.cpp
        PROPERTIES
        COMPILE_FLAGS "/Yc\"pch.h\""
    )
    
    set_source_files_properties(
        main.cpp
        MainWindow.cpp
        PROPERTIES
        COMPILE_FLAGS "/Yu\"pch.h\""
    )
endif()

# Сообщение об успешном конфигурировании
message(STATUS "TrayApp configured successfully")
message(STATUS "  Output: ${CMAKE_BINARY_DIR}/bin/TrayApp.exe")
