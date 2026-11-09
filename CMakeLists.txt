cmake_minimum_required(VERSION 3.24)

project(SoftwareGraphicsLabs LANGUAGES C CXX)

find_package(SDL3 CONFIG REQUIRED)

add_executable(graphics_lab
    src/main.cpp
    src/wolf/input/PlayerController.cpp
    src/wolf/raycast/Raycaster.cpp
    src/wolf/render/FrameBuffer.cpp
    src/wolf/render/SdlPresenter.cpp
    src/wolf/world/Map.cpp
)

target_compile_features(graphics_lab PRIVATE cxx_std_20)

target_include_directories(graphics_lab PRIVATE include)
target_link_libraries(graphics_lab PRIVATE SDL3::SDL3)

if(MSVC)
    target_compile_options(graphics_lab PRIVATE /W4)
else()
    target_compile_options(graphics_lab PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(graphics_lab PROPERTIES
    CXX_EXTENSIONS OFF
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

if(WIN32)
    add_custom_command(TARGET graphics_lab POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3>
            $<TARGET_FILE_DIR:graphics_lab>
    )
endif()

include(CTest)

if(BUILD_TESTING)
    add_executable(wolf_render_tests
        tests/render_tests.cpp
        src/wolf/raycast/Raycaster.cpp
        src/wolf/render/FrameBuffer.cpp
        src/wolf/world/Map.cpp
    )
    target_compile_features(wolf_render_tests PRIVATE cxx_std_20)
    target_include_directories(wolf_render_tests PRIVATE include)
    add_test(NAME wolf_render_tests COMMAND wolf_render_tests)
endif()
