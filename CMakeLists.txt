cmake_minimum_required(VERSION 3.16)

if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
endif()

project(TrayApp VERSION 1.0.0 LANGUAGES C CXX)

# Установка стандарта C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Глобальные параметры
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Поддержка разных платформ и архитектур
if(CMAKE_GENERATOR_PLATFORM MATCHES "ARM64")
    set(PLATFORM_ARCH "ARM64")
    message(STATUS "Building for ARM64 architecture")
elseif(CMAKE_GENERATOR_PLATFORM MATCHES "x64|Win64")
    set(PLATFORM_ARCH "x64")
    message(STATUS "Building for x64 architecture")
else()
    set(PLATFORM_ARCH "Unknown")
    message(STATUS "Building for ${CMAKE_GENERATOR_PLATFORM}")
endif()

# Параметры компиляции
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

    # Отключаем предупреждения о безопасных функциях
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    
    # Флаги оптимизации для Release
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /GL")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")
endif()

# Вывод информации о конфигурации
message(STATUS "")
message(STATUS "=== TrayApp Build Configuration ===")
message(STATUS "Generator: ${CMAKE_GENERATOR}")
message(STATUS "Architecture: ${PLATFORM_ARCH}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "====================================")
message(STATUS "")

add_subdirectory(src/TrayService)
add_subdirectory(src/TrayApp)

message(STATUS "TrayApp configuration completed successfully!")
