cmake_minimum_required(VERSION 3.24)

project(SoftwareGraphicsLabs LANGUAGES CXX)

add_executable(graphics_lab
    src/wolf/main.cpp
    src/wolf/app/Application.cpp
    src/wolf/world/Map.cpp
    src/wolf/input/PlayerController.cpp
    src/wolf/input/SdlInput.cpp
    src/wolf/raycast/Raycaster.cpp
)

target_include_directories(graphics_lab PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_features(graphics_lab PRIVATE cxx_std_20)

set_target_properties(graphics_lab PROPERTIES
    CXX_EXTENSIONS OFF
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_link_libraries(graphics_lab PRIVATE
        wolf_sdl
        SDL3::SDL3
)

add_library(wolf_render
    src/wolf/render/FrameBuffer.cpp
)

target_include_directories(wolf_render PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_features(wolf_render PUBLIC cxx_std_20)

enable_testing()

add_executable(wolf_render_test
    tests/framebuffer_test.cpp
)

target_link_libraries(wolf_render_test PRIVATE
    wolf_render
)

add_test(
    NAME wolf_render_test
    COMMAND wolf_render_test
)

add_executable(wolf_raster_test
    tests/framebuffer_raster_test.cpp
)

target_link_libraries(wolf_raster_test PRIVATE
    wolf_render
)

add_test(
    NAME wolf_raster_test
    COMMAND wolf_raster_test
)

find_package(SDL3 CONFIG REQUIRED)

add_library(wolf_sdl
    src/wolf/render/SdlPresenter.cpp
)

target_link_libraries(wolf_sdl
    PUBLIC wolf_render
    PRIVATE SDL3::SDL3
)

target_compile_features(wolf_sdl PUBLIC cxx_std_20)

add_executable(wolf_presenter_test
    tests/sdl_presenter_test.cpp
)

target_link_libraries(wolf_presenter_test PRIVATE
    wolf_sdl
    SDL3::SDL3
)

add_test(
    NAME wolf_presenter_test
    COMMAND wolf_presenter_test
)

add_executable(wolf_render_demo
    examples/wolf_render_demo.cpp
)

target_link_libraries(wolf_render_demo PRIVATE
    wolf_sdl
    SDL3::SDL3
)

add_executable(wolf_map_preview
    examples/wolf_map_preview.cpp
    src/wolf/world/Map.cpp
)

target_link_libraries(wolf_map_preview PRIVATE
    wolf_sdl
    SDL3::SDL3
)

add_executable(wolf_player_test
        tests/player_controller_test.cpp
        src/wolf/world/Map.cpp
        src/wolf/input/PlayerController.cpp
)

target_include_directories(wolf_player_test PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_features(wolf_player_test PRIVATE cxx_std_20)

add_test(
        NAME wolf_player_test
        COMMAND wolf_player_test
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    foreach(target
        graphics_lab
        wolf_render
        wolf_render_test
        wolf_raster_test
        wolf_sdl
        wolf_presenter_test
        wolf_render_demo
        wolf_map_preview
        wolf_player_test
    )
        target_compile_options(${target} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
        )
    endforeach()
endif()
