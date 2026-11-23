cmake_minimum_required(VERSION 3.24)

project(SoftwareGraphicsLabs LANGUAGES CXX)

find_package(SDL3 CONFIG REQUIRED)
find_package(PNG REQUIRED)

add_library(wolf_render STATIC
    src/wolf/world/Map.cpp
    src/wolf/render/framebuffer.cpp
    src/wolf/raycast/Raycaster.cpp
    src/wolf/texture/TextureStore.cpp
    src/wolf/sprite/SpriteRenderer.cpp
    src/wolf/effects/Fog.cpp
)
target_include_directories(wolf_render PUBLIC include)
target_compile_features(wolf_render PUBLIC cxx_std_20)
target_link_libraries(wolf_render PUBLIC PNG::PNG)
target_compile_options(wolf_render PRIVATE -Wall -Wextra -Wpedantic -O3)
set_target_properties(wolf_render PROPERTIES CXX_EXTENSIONS OFF)

add_executable(graphics_lab
    src/wolf/main.cpp
    src/wolf/app/Application.cpp
    src/wolf/input/PlayerController.cpp
    src/wolf/render/SdlPresenter.cpp
)
target_link_libraries(graphics_lab PRIVATE wolf_render SDL3::SDL3)
target_compile_definitions(graphics_lab PRIVATE WOLF_ASSETS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets/wolf")
target_compile_options(graphics_lab PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -O3
)

set_target_properties(graphics_lab PROPERTIES
    CXX_EXTENSIONS OFF
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

include(CTest)
if(BUILD_TESTING)
    add_executable(lab02_render_tests tests/Lab02RenderTests.cpp)
    target_link_libraries(lab02_render_tests PRIVATE wolf_render)
    target_compile_definitions(lab02_render_tests PRIVATE
        LAB02_FIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/lab02")
    target_compile_options(lab02_render_tests PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME lab02_render_tests COMMAND lab02_render_tests)

    add_executable(lab02_demo
        tests/Lab02Demo.cpp
        src/wolf/render/SdlPresenter.cpp
        src/wolf/input/PlayerController.cpp)
    target_link_libraries(lab02_demo PRIVATE wolf_render SDL3::SDL3)
    target_compile_definitions(lab02_demo PRIVATE
        LAB02_FIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/lab02"
        WOLF_ASSETS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets/wolf")
    target_compile_options(lab02_demo PRIVATE -Wall -Wextra -Wpedantic)
    set_target_properties(lab02_demo PROPERTIES
        CXX_EXTENSIONS OFF RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

    add_executable(lab02_benchmark tests/Lab02Benchmark.cpp src/wolf/render/SdlPresenter.cpp)
    target_link_libraries(lab02_benchmark PRIVATE wolf_render SDL3::SDL3)
    target_compile_definitions(lab02_benchmark PRIVATE
        LAB02_FIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/lab02"
        WOLF_ASSETS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets/wolf")
    target_compile_options(lab02_benchmark PRIVATE -Wall -Wextra -Wpedantic)
    set_target_properties(lab02_benchmark PROPERTIES
        CXX_EXTENSIONS OFF RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endif()
