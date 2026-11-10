cmake_minimum_required(VERSION 3.16)

project(graphics_lab LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)

add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp" "src/*.h"  "src/menu*.cpp" "src/menu*.h")

add_executable(graphics_lab
	${SOURCES}
)

target_link_libraries(graphics_lab PRIVATE SDL3::SDL3-static)

option(ENABLE_SANITIZERS "Enable Address and Undefined Behavior sanitizers" ON)
