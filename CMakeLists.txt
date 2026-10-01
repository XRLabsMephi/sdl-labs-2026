cmake_minimum_required(VERSION 3.24)

project(SoftwareGraphicsLabs LANGUAGES CXX)

add_executable(graphics_lab
        src/wolf/main.cpp
        include/wolf/core/Color.hpp
        include/wolf/core/Vector2.hpp
        include/wolf/world/Map.hpp
        src/wolf/world/Map.cpp
        include/wolf/world/Player.hpp
        include/wolf/world/Player.hpp
        include/wolf/input/InputState.hpp
        include/wolf/input/PlayerController.hpp
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

include(FetchContent)

FetchContent_Declare(
        SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG        release-3.2.0
        GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(SDL3)

target_link_libraries(graphics_lab PRIVATE SDL3::SDL3)

add_custom_command(TARGET graphics_lab POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL3::SDL3>
        $<TARGET_FILE_DIR:graphics_lab>
)

target_include_directories(graphics_lab PRIVATE
        include
        include/wolf
)

target_compile_definitions(graphics_lab PRIVATE
        ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets"
)