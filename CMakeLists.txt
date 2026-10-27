cmake_minimum_required(VERSION 3.24)

project(SoftwareGraphicsLabs LANGUAGES CXX)

add_executable(graphics_lab
    src/main.cpp
)

target_compile_features(graphics_lab PRIVATE cxx_std_20)

target_compile_options(graphics_lab PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

set_target_properties(graphics_lab PROPERTIES
    CXX_EXTENSIONS OFF
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
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

target_link_libraries(wolf_render_test PRIVATE wolf_render)

add_test(NAME wolf_render_test COMMAND wolf_render_test)
