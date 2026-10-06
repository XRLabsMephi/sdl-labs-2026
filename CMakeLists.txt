cmake_minimum_required(VERSION 3.24)

project(SoftwareGraphicsLabs LANGUAGES CXX)

find_package(SDL3 REQUIRED CONFIG)

set(HEADERS
        include/wolf/app/Application.h
        include/wolf/core/Vec2.h
        include/wolf/core/Color.h
        include/wolf/world/Map.h
        include/wolf/world/Player.h
        include/wolf/render/FrameBuffer.h
        include/wolf/render/SdlPresenter.h
        include/wolf/raycast/Raycaster.h
        include/wolf/raycast/RayHit.h
        include/wolf/input/InputState.h
        include/wolf/input/PlayerController.h
)

set(SOURCES
        src/wolf/main.cpp
        src/wolf/app/Application.cpp
        src/wolf/input/PlayerController.cpp
        src/wolf/raycast/Raycaster.cpp
        src/wolf/render/FrameBuffer.cpp
        src/wolf/render/SdlPresenter.cpp
        src/wolf/world/Map.cpp
)

add_executable(graphics_lab ${HEADERS} ${SOURCES})

target_include_directories(graphics_lab PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(graphics_lab PRIVATE SDL3::SDL3)

target_compile_features(graphics_lab PRIVATE cxx_std_20)

target_compile_options(graphics_lab PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic>
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->
)

set_target_properties(graphics_lab PROPERTIES
    CXX_EXTENSIONS OFF
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
