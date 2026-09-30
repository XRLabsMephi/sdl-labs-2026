#pragma once
#include <vector>
#include <string>

class Map {
    int width_ = 0, height_ = 0;
    std::vector<int> map_;

    bool is_in_map(int x, int y);

public:
    Map() = default;

    bool load_from_file(const std::string& path);
    bool is_wall(int x, int y);
    bool is_wall(double x, double y);
    int at(int x, int y);

    [[nodiscard]] int get_width() const { return width_; }
    [[nodiscard]] int get_height() const { return height_; }
};