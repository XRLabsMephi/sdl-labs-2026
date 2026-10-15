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
