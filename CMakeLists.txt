cmake_minimum_required(VERSION 3.20)
project(wolf_lab01 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL3 CONFIG REQUIRED)

add_executable(wolf_lab01
    src/wolf/main.cpp
    src/wolf/app/Application.cpp
    src/wolf/world/Map.cpp
    src/wolf/input/PlayerController.cpp
    src/wolf/render/FrameBuffer.cpp
    src/wolf/render/SdlPresenter.cpp
    src/wolf/raycast/Raycaster.cpp
)

target_include_directories(wolf_lab01 PRIVATE include)
target_link_libraries(wolf_lab01 PRIVATE SDL3::SDL3)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
