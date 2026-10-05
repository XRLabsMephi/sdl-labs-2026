cmake_minimum_required(VERSION 3.24)

project(SoftwareGraphicsLabs LANGUAGES CXX)

find_package(SDL3 CONFIG REQUIRED)

add_executable(graphics_lab
    src/wolf/main.cpp
    src/wolf/app/Application.cpp
    src/wolf/world/Map.cpp
    src/wolf/input/PlayerController.cpp
    src/wolf/render/framebuffer.cpp
    src/wolf/render/SdlPresenter.cpp
)

target_include_directories(graphics_lab PRIVATE
    include
)

target_compile_features(graphics_lab PRIVATE cxx_std_20)

target_link_libraries(graphics_lab PRIVATE
    SDL3::SDL3
)
target_compile_options(graphics_lab PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

set_target_properties(graphics_lab PROPERTIES
    CXX_EXTENSIONS OFF
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
