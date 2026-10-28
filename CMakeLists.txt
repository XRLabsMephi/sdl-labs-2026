cmake_minimum_required(VERSION 3.24)

project(SoftwareGraphicsLabs LANGUAGES CXX)

include(FetchContent)
set(SDL_SHARED OFF CACHE BOOL "Build SDL shared library")
set(SDL_STATIC ON CACHE BOOL "Build SDL static library")
set(SDL_TEST_LIBRARY OFF CACHE BOOL "Build SDL test library")
set(SDL_TESTS OFF CACHE BOOL "Build SDL tests")
set(SDL_EXAMPLES OFF CACHE BOOL "Build SDL examples")
set(SDL_INSTALL OFF CACHE BOOL "Install SDL")
FetchContent_Declare(SDL3
        URL https://www.libsdl.org/release/SDL3-3.4.18.tar.gz
        URL_HASH SHA256=9c75cf16330322c217dedd2e0609f1124f1b54b8633e763467b4684d0f4334a3
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        TIMEOUT 60
        TLS_VERIFY TRUE
)
FetchContent_MakeAvailable(SDL3)

add_library(wolf_world STATIC
        src/wolf/world/Map.cpp
        src/wolf/input/PlayerController.cpp
)

target_include_directories(wolf_world PUBLIC include)
target_compile_features(wolf_world PUBLIC cxx_std_20)
target_compile_options(wolf_world PRIVATE -Wall -Wextra -Wpedantic)
set_target_properties(wolf_world PROPERTIES CXX_EXTENSIONS OFF)

add_library(wolf_raycast STATIC src/wolf/raycast/Raycaster.cpp)
target_link_libraries(wolf_raycast PUBLIC wolf_world)
target_compile_options(wolf_raycast PRIVATE -Wall -Wextra -Wpedantic)
set_target_properties(wolf_raycast PROPERTIES CXX_EXTENSIONS OFF)

add_library(wolf_render STATIC
        src/wolf/render/FrameBuffer.cpp
        src/wolf/render/SoftwareRenderer.cpp
        src/wolf/render/SdlPresenter.cpp
)
target_include_directories(wolf_render PUBLIC include)
target_link_libraries(wolf_render PUBLIC wolf_raycast SDL3::SDL3)
target_compile_features(wolf_render PUBLIC cxx_std_20)
target_compile_options(wolf_render PRIVATE -Wall -Wextra -Wpedantic)
set_target_properties(wolf_render PROPERTIES CXX_EXTENSIONS OFF)

add_library(wolf_resources STATIC
        src/wolf/texture/BmpLoader.cpp
        src/wolf/texture/TextureStore.cpp
        src/wolf/world/SceneResources.cpp
)
target_include_directories(wolf_resources PUBLIC include)
target_compile_features(wolf_resources PUBLIC cxx_std_20)
target_link_libraries(wolf_resources PUBLIC wolf_world PRIVATE SDL3::SDL3)
target_compile_options(wolf_resources PRIVATE -Wall -Wextra -Wpedantic)
set_target_properties(wolf_resources PROPERTIES CXX_EXTENSIONS OFF)

add_library(wolf_app STATIC src/wolf/app/Application.cpp)
target_link_libraries(wolf_app PUBLIC wolf_render)
target_compile_options(wolf_app PRIVATE -Wall -Wextra -Wpedantic)
set_target_properties(wolf_app PROPERTIES CXX_EXTENSIONS OFF)

add_executable(graphics_lab
        src/wolf/main.cpp
)

target_link_libraries(graphics_lab PRIVATE wolf_app)
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

add_custom_target(wolf_assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/assets/wolf"
            "$<TARGET_FILE_DIR:graphics_lab>/assets/wolf"
    COMMENT "Copying Wolf scene resources"
)
add_dependencies(graphics_lab wolf_assets)

include(CTest)

if(BUILD_TESTING)
    add_executable(texture_tests tests/wolf/texture/TextureTests.cpp)
    target_include_directories(texture_tests PRIVATE include)
    target_compile_features(texture_tests PRIVATE cxx_std_20)
    target_compile_options(texture_tests PRIVATE -Wall -Wextra -Wpedantic)
    set_target_properties(texture_tests PROPERTIES CXX_EXTENSIONS OFF)
    add_test(NAME wolf_texture COMMAND texture_tests)

    add_executable(texture_store_tests tests/wolf/texture/TextureStoreTests.cpp)
    target_link_libraries(texture_store_tests PRIVATE wolf_resources)
    target_compile_options(texture_store_tests PRIVATE -Wall -Wextra -Wpedantic)
    set_target_properties(texture_store_tests PROPERTIES CXX_EXTENSIONS OFF)
    add_test(NAME wolf_texture_store COMMAND texture_store_tests)

    add_executable(scene_resources_tests tests/wolf/world/SceneResourcesTests.cpp)
    target_link_libraries(scene_resources_tests PRIVATE wolf_resources wolf_raycast)
    target_compile_options(scene_resources_tests PRIVATE -Wall -Wextra -Wpedantic)
    set_target_properties(scene_resources_tests PROPERTIES
            CXX_EXTENSIONS OFF
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
    add_dependencies(scene_resources_tests wolf_assets)
    add_test(NAME wolf_scene_resources COMMAND scene_resources_tests)
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/resource-test-cwd")
    add_test(NAME wolf_scene_resources_other_directory COMMAND scene_resources_tests)
    set_tests_properties(wolf_scene_resources_other_directory PROPERTIES
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/resource-test-cwd"
    )

    add_executable(bmp_loader_tests tests/wolf/texture/BmpLoaderTests.cpp)
    target_link_libraries(bmp_loader_tests PRIVATE wolf_resources)
    target_compile_options(bmp_loader_tests PRIVATE -Wall -Wextra -Wpedantic)
    set_target_properties(bmp_loader_tests PROPERTIES CXX_EXTENSIONS OFF)
    add_test(NAME wolf_bmp_loader COMMAND bmp_loader_tests)

    add_executable(map_tests tests/wolf/world/MapTests.cpp)
    target_link_libraries(map_tests PRIVATE wolf_world)
    target_compile_options(map_tests PRIVATE -Wall -Wextra -Wpedantic)
    set_target_properties(map_tests PROPERTIES CXX_EXTENSIONS OFF)
    add_test(NAME wolf_map COMMAND map_tests)

    add_executable(player_controller_tests tests/wolf/input/PlayerControllerTests.cpp)
    target_link_libraries(player_controller_tests PRIVATE wolf_world)
    target_compile_options(player_controller_tests PRIVATE -Wall -Wextra -Wpedantic)
    set_target_properties(player_controller_tests PROPERTIES CXX_EXTENSIONS OFF)
    add_test(NAME wolf_player_controller COMMAND player_controller_tests)

    add_executable(raycaster_tests tests/wolf/raycast/RaycasterTests.cpp)
    target_link_libraries(raycaster_tests PRIVATE wolf_raycast)
    target_compile_options(raycaster_tests PRIVATE -Wall -Wextra -Wpedantic)
    set_target_properties(raycaster_tests PROPERTIES CXX_EXTENSIONS OFF)
    add_test(NAME wolf_raycaster COMMAND raycaster_tests)

    add_executable(application_tests tests/wolf/app/ApplicationTests.cpp)
    target_link_libraries(application_tests PRIVATE wolf_app)
    target_compile_options(application_tests PRIVATE -Wall -Wextra -Wpedantic)
    set_target_properties(application_tests PROPERTIES CXX_EXTENSIONS OFF)
    add_test(NAME wolf_application COMMAND application_tests)
    set_tests_properties(wolf_application PROPERTIES
            ENVIRONMENT "SDL_VIDEODRIVER=dummy"
            TIMEOUT 15
    )
    add_test(NAME wolf_application_other_directory COMMAND application_tests)
    set_tests_properties(wolf_application_other_directory PROPERTIES
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/resource-test-cwd"
            ENVIRONMENT "SDL_VIDEODRIVER=dummy"
            TIMEOUT 15
    )
    add_executable(software_renderer_tests tests/wolf/render/SoftwareRendererTests.cpp)
    target_link_libraries(software_renderer_tests PRIVATE wolf_render)
    target_compile_options(software_renderer_tests PRIVATE -Wall -Wextra -Wpedantic)
    set_target_properties(software_renderer_tests PROPERTIES CXX_EXTENSIONS OFF)
    add_test(NAME wolf_software_renderer COMMAND software_renderer_tests)

    add_executable(render_pipeline_tests tests/wolf/render/RenderPipelineTests.cpp)
    target_link_libraries(render_pipeline_tests PRIVATE wolf_render)
    target_compile_options(render_pipeline_tests PRIVATE -Wall -Wextra -Wpedantic)
    set_target_properties(render_pipeline_tests PROPERTIES CXX_EXTENSIONS OFF)
    add_test(NAME wolf_render_pipeline COMMAND render_pipeline_tests)

    add_executable(sdl_presenter_tests tests/wolf/render/SdlPresenterTests.cpp)
    target_link_libraries(sdl_presenter_tests PRIVATE wolf_render)
    target_compile_options(sdl_presenter_tests PRIVATE -Wall -Wextra -Wpedantic)
    set_target_properties(sdl_presenter_tests PROPERTIES CXX_EXTENSIONS OFF)
    add_test(NAME wolf_sdl_presenter COMMAND sdl_presenter_tests)
    set_tests_properties(wolf_sdl_presenter PROPERTIES ENVIRONMENT "SDL_VIDEODRIVER=dummy")
endif()
