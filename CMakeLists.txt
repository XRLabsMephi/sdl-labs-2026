cmake_minimum_required(VERSION 3.24)

project(SoftwareGraphicsLabs LANGUAGES CXX)

add_library(wolf_world STATIC
    src/wolf/world/Map.cpp
)

target_include_directories(wolf_world PUBLIC include)
target_compile_features(wolf_world PUBLIC cxx_std_20)
target_compile_options(wolf_world PRIVATE -Wall -Wextra -Wpedantic)
set_target_properties(wolf_world PROPERTIES CXX_EXTENSIONS OFF)

add_executable(graphics_lab
    src/main.cpp
)

target_link_libraries(graphics_lab PRIVATE wolf_world)
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
endif()
