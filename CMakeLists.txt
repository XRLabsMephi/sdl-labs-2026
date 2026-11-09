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

add_executable(laba ${SOURCES}
        src/wolf/render/FrameBuffer.cpp
        src/wolf/render/SdlPresenter.cpp
        src/wolf/app/Application.cpp
        src/wolf/world/Map.cpp
        src/wolf/input/PlayerController.cpp
        src/wolf/raycast/Raycaster.cpp
)
target_link_libraries(laba PRIVATE SDL3::SDL3)
