#pragma once

#include <array>
#include <filesystem>
#include <string>
#include <render/ViewSettings.h>

namespace config {
    struct ProjectSettings {
        std::string title;
        int window_width{}, window_height{}, frame_width{}, frame_height{};
        bool resizable{}, relative_mouse{}, show_fps{};
        std::string map, texture_mapping;
        double field_of_view_deg{}, eye_height{}, player_radius{};
        double move_speed{}, rotation_speed{}, mouse_sensitivity{}, max_movement_step{};
        core::Color fog_color;
        float fog_start{}, fog_end{};
        ViewSettings view;
        std::array<std::string, 6> movement_keys;
        std::string exit_key;
    };

    ProjectSettings load(const std::filesystem::path& path);
}
