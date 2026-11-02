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

option(WOLF_BUILD_APP "Собрать приложение SDL3" ON)

if(WOLF_BUILD_APP)
    find_package(SDL3 3.2 REQUIRED CONFIG)
    add_library(wolf_app
        src/wolf/app/Application.cpp
        src/wolf/render/SdlPresenter.cpp
        src/wolf/texture/TextureStore.cpp
    )
    target_link_libraries(wolf_app PUBLIC wolf_core SDL3::SDL3)
    wolf_target(wolf_app)

    add_executable(graphics_lab src/wolf/main.cpp)
    target_link_libraries(graphics_lab PRIVATE wolf_app)
    wolf_target(graphics_lab)

    add_custom_target(wolf_assets ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:graphics_lab>/assets"
    )
    add_dependencies(graphics_lab wolf_assets)

    if(WIN32 AND TARGET SDL3::SDL3-shared)
        add_custom_command(TARGET graphics_lab POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<TARGET_FILE:SDL3::SDL3-shared>"
                "$<TARGET_FILE_DIR:graphics_lab>"
        )
    endif()
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(wolf_tests tests/wolf_tests.cpp)
    target_link_libraries(wolf_tests PRIVATE wolf_core)
    wolf_target(wolf_tests)
    add_test(NAME wolf_core COMMAND wolf_tests)
endif()
