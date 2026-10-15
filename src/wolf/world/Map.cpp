#include "world/Map.hpp"

#include <fstream>
#include <sstream>
#include <string>
#include <cmath>

void Map::clear() {
    width_ = 0;
    height_ = 0;
    map_.clear();
}

bool Map::load_from_file(const std::string &path) {
    clear();

    std::ifstream file(path);
    if (!file) { return false; }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        height_++;

        std::istringstream iss(line);

        int row_width = 0;

        int x;
        while (iss >> x) {
            map_.push_back(x);
            row_width++;
        }

        if (width_ == 0) { width_ = row_width; }
        else if (row_width != width_) {
            clear();
            return false;
        }
    }

    if (map_.empty()) {
        clear();
        return false;
    }
    return true;
}

bool Map::is_in_map(int x, int y) const {
    return (
            x >= 0 &&
            x < width_ &&
            y >= 0 &&
            y < height_
    );
}

int Map::at(int x, int y) const {
    if (!is_in_map(x, y)) return -1;

    return map_[y * width_ + x];
}

bool Map::is_wall(int x, int y) const {
    if (!is_in_map(x, y)) return true; // за границей карты - стена
    return at(x, y) != 0;
}

bool Map::is_wall(double x, double y) const {
    int x_ = std::floor(x);
    int y_ = std::floor(y);
    return is_wall(x_, y_);
}