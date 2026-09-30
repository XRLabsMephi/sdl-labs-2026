#include "world/Map.hpp"

#include <fstream>
#include <sstream>
#include <string>

bool Map::load_from_file(const std::string &path) {
    std::ifstream file(path);
    if (!file) { return false; }

    height_ = 0;
    width_ = -1;

    std::string line;
    while (std::getline(file, line)) {
        height_++;

        std::istringstream iss(line);

        int row_width = 0;

        int x;
        while (iss >> x) {
            map_.push_back(x);
            row_width++;
        }

        if (width_ == -1) { width_ = row_width; }
        else if (row_width != width_) return false;
    }

    if (map_.empty()) return false;
    return true;
}