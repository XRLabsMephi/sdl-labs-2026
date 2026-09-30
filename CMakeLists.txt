cmake_minimum_required(VERSION 3.24)
project(SoftwareGraphicsLabs LANGUAGES CXX)

option(GRAPHICS_LAB_VENDORED "Use vendored libraries" OFF)

if(GRAPHICS_LAB_VENDORED)
    add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
else()
    find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)
endif()

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

add_executable(graphics_lab
    src/core/application.cc
    src/core/application.h

    src/core/config.cc
    src/core/config.h

    src/math/vector.h

    src/world/map.cc
    src/world/map.h
    src/world/tile.h

    src/main.cc
)

target_include_directories(graphics_lab PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_compile_options(graphics_lab PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

target_link_libraries(graphics_lab PRIVATE SDL3::SDL3)
