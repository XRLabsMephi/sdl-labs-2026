#pragma once

#include <string>
#include <vector>

namespace wolf {

class Map {
public:
    explicit Map(std::vector<std::vector<int>> cells);

    static Map load(const std::string& filename);

    int get_width() const;
    int get_height() const;

    bool in_bounds(int x, int y) const;
    int wall_id(int x, int y) const;
    bool is_wall(int x, int y) const;

private:
    std::vector<std::vector<int>> cells_;
    int width_ = 0;
    int height_ = 0;
};

}