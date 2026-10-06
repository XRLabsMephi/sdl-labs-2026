cmake_minimum_required(VERSION 3.16)

project(graphics_lab LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)

add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)

add_executable(graphics_lab
        src/main.cpp
        src/playfield.cpp
        src/board.cpp
        src/playfield.h
        src/board.h
)

target_link_libraries(graphics_lab PRIVATE SDL3::SDL3-static)