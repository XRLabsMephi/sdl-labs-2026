find_package(GTest QUIET)

if(NOT TARGET GTest::gtest_main)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.15.2
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

add_executable(graphics_lab_tests
    world/map_test.cc
)

target_link_libraries(graphics_lab_tests PRIVATE
    graphics_lab_core
    GTest::gtest_main
)

target_compile_definitions(graphics_lab_tests PRIVATE
    TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/test_data"
    PROJECT_ROOT_DIR="${PROJECT_SOURCE_DIR}"
)

include(GoogleTest)
gtest_discover_tests(graphics_lab_tests)
