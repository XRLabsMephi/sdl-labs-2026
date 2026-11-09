#pragma once
#include <vector>
#include <string>
#include <optional>
#include "core/Vector2.hpp"

class Map {
    int width_ = 0, height_ = 0;
    std::vector<int> map_;

    [[nodiscard]] bool is_in_map(int x, int y) const;
    void clear();

public:
    Map() = default;

    bool load_from_file(const std::string& path);
    [[nodiscard]] bool is_wall(int x, int y) const;
    [[nodiscard]] bool is_wall(double x, double y) const;
    [[nodiscard]] int at(int x, int y) const;

    [[nodiscard]] int get_width() const { return width_; }
    [[nodiscard]] int get_height() const { return height_; }

    [[nodiscard]] std::optional<std::pair<int, int>> find_nearest_free_cell(double x, double y) const;
    bool circle_hits_wall(Vector2 center, double radius) const;
};