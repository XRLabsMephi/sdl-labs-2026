cmake_minimum_required(VERSION 3.24)

project(SoftwareGraphicsLabs LANGUAGES CXX)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

function(wolf_target target)
    target_compile_features(${target} PUBLIC cxx_std_20)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /utf-8)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

add_library(wolf_core
    src/wolf/world/Map.cpp
    src/wolf/input/PlayerController.cpp
    src/wolf/render/FrameBuffer.cpp
    src/wolf/raycast/Raycaster.cpp
)
target_include_directories(wolf_core PUBLIC include)
wolf_target(wolf_core)

add_executable(graphics_lab src/main.cpp)
target_link_libraries(graphics_lab PRIVATE wolf_core)
wolf_target(graphics_lab)
