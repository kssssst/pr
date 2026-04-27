cmake_minimum_required(VERSION 3.16)

project(TrayService)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Добавляем исходные файлы
add_executable(TrayService WIN32
    main.cpp
)

# Добавляем необходимые библиотеки Windows
target_link_libraries(TrayService
    advapi32
    shell32
    user32
)

# Параметры компилятора
if(MSVC)
    target_compile_options(TrayService PRIVATE /W4)
    target_compile_definitions(TrayService PRIVATE _UNICODE UNICODE)
endif()

# Выходной файл
set_target_properties(TrayService PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
