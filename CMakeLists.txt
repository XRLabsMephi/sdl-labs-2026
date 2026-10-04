cmake_minimum_required(VERSION 3.24)

project(SoftwareGraphicsLabs LANGUAGES CXX)

# --- Настройки SDL (vendoring) ---
# Отключаем сборку тестов и примеров SDL, чтобы не тратить время при компиляции
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)

# Принудительно включаем сборку динамической (SHARED) библиотеки
set(SDL_SHARED ON CACHE BOOL "" FORCE)
set(SDL_STATIC OFF CACHE BOOL "" FORCE)

# Добавляем SDL из вашей локальной папки
add_subdirectory(vendored/SDL)

# SDL's macOS export-list option uses SHELL without quoting the source path.
# Quote it here so linking also works when the project directory contains spaces.
if(APPLE AND TARGET SDL3-shared)
    get_target_property(sdl_link_options SDL3-shared LINK_OPTIONS)
    if(sdl_link_options)
        list(TRANSFORM sdl_link_options REPLACE
            "^SHELL:(-Wl,-exported_symbols_list,.*)$" "SHELL:\"\\1\"")
        set_property(TARGET SDL3-shared PROPERTY LINK_OPTIONS "${sdl_link_options}")
    endif()
endif()

# --- Наш исполняемый файл ---
add_executable(graphics_lab
    src/main.cpp
)

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

# --- Связывание с SDL ---
# Если вы используете актуальную ветку (SDL3), цель называется SDL3::SDL3
# Если вы переключили репозиторий на ветку SDL2, замените везде SDL3::SDL3 на SDL2::SDL2
target_link_libraries(graphics_lab PRIVATE SDL3::SDL3)

# --- Автоматическое копирование DLL (для Windows) ---
# Копирует собранную SDL3.dll в вашу папку ${CMAKE_BINARY_DIR}/bin сразу после сборки
if(WIN32)
    add_custom_command(TARGET graphics_lab POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL3::SDL3>
        $<TARGET_FILE_DIR:graphics_lab>
    )
endif()
