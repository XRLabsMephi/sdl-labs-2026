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

add_library(wolf_app STATIC src/wolf/app/Application.cpp)
target_link_libraries(wolf_app PUBLIC wolf_world SDL3::SDL3)
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

include(CTest)

if(BUILD_TESTING)
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

    add_executable(application_tests tests/wolf/app/ApplicationTests.cpp)
    target_link_libraries(application_tests PRIVATE wolf_app)
    target_compile_options(application_tests PRIVATE -Wall -Wextra -Wpedantic)
    set_target_properties(application_tests PROPERTIES CXX_EXTENSIONS OFF)
    add_test(NAME wolf_application COMMAND application_tests)
    set_tests_properties(wolf_application PROPERTIES
        ENVIRONMENT "SDL_VIDEODRIVER=dummy"
        TIMEOUT 15
    )
endif()
