#include <config/ProjectSettings.h>
#include <debug/Logger.h>
#include <cmath>
#include <fstream>
#include <locale>
#include <sstream>
#include <type_traits>
#include <unordered_map>

namespace config {
    ProjectSettings load(const std::filesystem::path& path) {
        using logging::Error;
        using logging::ErrorCode;
        std::ifstream file(path);
        if (!file) throw Error(ErrorCode::ConfigOpen, "Cannot open " + path.string());

        // Keys are unique; blank lines and full-line comments are allowed.
        std::unordered_map<std::string, std::string> entries;
        std::string line;
        for (int number = 1; std::getline(file, line); ++number) {
            std::istringstream row(line);
            std::string key;
            if (!(row >> key) || key.starts_with('#')) continue;
            std::string value;
            std::getline(row >> std::ws, value);
            if (!entries.emplace(key, value).second) {
                throw Error(ErrorCode::ConfigValue, "Duplicate key " + key + " on line " + std::to_string(number));
            }
        }
        if (file.bad()) throw Error(ErrorCode::ConfigOpen, "Cannot read " + path.string());

        const auto read = [&]<class T>(const char* key, T& out) {
            const auto it = entries.find(key);
            if (it == entries.end()) throw Error(ErrorCode::ConfigValue, std::string("Missing key: ") + key);

            std::istringstream value(it->second);
            value.imbue(std::locale::classic());
            bool ok = false;
            if constexpr (std::is_same_v<T, std::string>) {
                const auto end = it->second.find_last_not_of(" \r\t");
                ok = end != std::string::npos;
                if (ok) out = it->second.substr(0, end + 1);
            } else if constexpr (std::is_same_v<T, core::Color>) {
                int r, g, b;
                if ((value >> r >> g >> b) && r >= 0 && r <= 255 && g >= 0 && g <= 255 && b >= 0 && b <= 255) {
                    out = core::Color(static_cast<Uint8>(r), static_cast<Uint8>(g), static_cast<Uint8>(b));
                    ok = (value >> std::ws).eof();
                }
            } else {
                ok = static_cast<bool>(value >> std::boolalpha >> out) && (value >> std::ws).eof();
                if constexpr (std::is_floating_point_v<T>) ok = ok && std::isfinite(out);
            }
            if (!ok) throw Error(ErrorCode::ConfigValue, std::string("Invalid value for ") + key);
            entries.erase(it);
        };

        ProjectSettings s;
        read("window_title", s.title);
        read("window_width", s.window_width);
        read("window_height", s.window_height);
        read("frame_width", s.frame_width);
        read("frame_height", s.frame_height);
        read("window_resizable", s.resizable);
        read("relative_mouse", s.relative_mouse);
        read("show_fps", s.show_fps);
        read("map", s.map);
        read("texture_mapping", s.texture_mapping);
        read("field_of_view_deg", s.field_of_view_deg);
        read("eye_height", s.eye_height);
        read("player_radius", s.player_radius);
        read("move_speed", s.move_speed);
        read("rotation_speed", s.rotation_speed);
        read("mouse_sensitivity", s.mouse_sensitivity);
        read("max_movement_step", s.max_movement_step);
        read("fish_eye", s.view.fish_eye);
        read("angle_projection", s.view.angle_projection);
        read("wall_color", s.view.wall_color);
        read("ground_color", s.view.ground_color);
        read("sky_color", s.view.sky_color);
        read("minimal_brightness", s.view.minimal_brightness);
        read("darkening_distance", s.view.darkening_distance);
        read("fog_color", s.fog_color);
        read("fog_start", s.fog_start);
        read("fog_end", s.fog_end);
        constexpr const char* key_names[] = {
            "forward_key", "backward_key", "left_key", "right_key", "turn_left_key", "turn_right_key"
        };
        for (std::size_t i = 0; i < s.movement_keys.size(); ++i) read(key_names[i], s.movement_keys[i]);
        read("exit_key", s.exit_key);
        if (!entries.empty()) throw Error(ErrorCode::ConfigValue, "Unknown key: " + entries.begin()->first);

        const auto require = [](bool condition, const char* message) {
            if (!condition) throw Error(ErrorCode::ConfigValue, message);
        };
        require(s.window_width > 0 && s.window_height > 0 && s.frame_width > 1 && s.frame_height > 1,
                "Invalid window or frame size");
        require(s.field_of_view_deg >= 1.0 && s.field_of_view_deg <= 179.0, "FOV must be in [1, 179]");
        require(s.eye_height > 0.0 && s.eye_height < 1.0, "Eye height must be between floor (0) and wall top (1)");
        require(s.player_radius > 0.0 && s.move_speed >= 0.0 && s.rotation_speed >= 0.0 &&
                s.mouse_sensitivity >= 0.0 && s.max_movement_step > 0.0, "Invalid player or movement settings");
        require(s.view.minimal_brightness >= 0.0 && s.view.minimal_brightness <= 1.0 &&
                s.view.darkening_distance > 0.0, "Invalid light settings");
        require(s.fog_start >= 0.0f && s.fog_start < s.fog_end, "Fog requires 0 <= start < end");
        return s;
    }
}
