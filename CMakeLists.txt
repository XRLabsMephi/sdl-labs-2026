cmake_minimum_required(VERSION 3.24)

project(SoftwareGraphicsLabs LANGUAGES C CXX)

include(FetchContent)

set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
        SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG release-3.4.18
        GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(SDL3)

add_executable(graphics_lab
        src/main.cpp
        src/wolf/core/Vec2.cpp
        src/wolf/core/Color.cpp
        src/wolf/world/Map.cpp
        src/wolf/render/FrameBuffer.cpp
        src/wolf/Raycaster.cpp
        src/wolf/input/PlayerController.cpp
        src/wolf/render/SdlPresenter.cpp
        src/wolf/app/Application.cpp
        include/wolf/core/Vec2.hpp
        include/wolf/core/Color.hpp
        include/wolf/world/Map.hpp
        include/wolf/world/Player.hpp
        include/wolf/render/FrameBuffer.hpp
        include/wolf/raycast/RayHit.hpp
        include/wolf/raycast/Raycaster.hpp
        include/wolf/input/InputState.hpp
        include/wolf/input/PlayerController.hpp
        include/wolf/render/SdlPresenter.hpp
        include/wolf/app/Application.hpp
)

target_include_directories(graphics_lab PRIVATE include)

target_link_libraries(graphics_lab PRIVATE SDL3::SDL3)

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

add_executable(test
        test/test.cpp
        src/wolf/world/Map.cpp
        src/wolf/core/Color.cpp
        src/wolf/core/Vec2.cpp
        src/wolf/render/FrameBuffer.cpp
        src/wolf/Raycaster.cpp
)

target_include_directories(test PRIVATE include)

target_compile_features(test PRIVATE cxx_std_20)

add_custom_target(copy_resources
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/resources"
        "$<TARGET_FILE_DIR:graphics_lab>/resources"
        VERBATIM
)

add_dependencies(graphics_lab copy_resources)