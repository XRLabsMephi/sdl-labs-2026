cmake_minimum_required(VERSION 3.20)
project(wolf_raycaster CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

include(FetchContent)
FetchContent_Declare(
        SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG main
)
FetchContent_MakeAvailable(SDL3)

file(GLOB_RECURSE SOURCES "src/wolf/*.cpp")

add_executable(wolf_raycaster ${SOURCES})

target_link_libraries(wolf_raycaster PRIVATE SDL3::SDL3)