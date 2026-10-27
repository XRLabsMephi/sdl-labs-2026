#include "wolf/world/Map.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <utility>

Map::Map(std::vector<std::vector<int>> cells)
    : cells_(std::move(cells)) {

    if (cells_.empty() || cells_[0].empty()) {
        throw std::invalid_argument("Map must not be empty");
    }

    height_ = static_cast<int>(cells_.size());
    width_ = static_cast<int>(cells_[0].size());
    for (const auto& row : cells_) {
        if (static_cast<int>(row.size()) != width_) {
            throw std::invalid_argument("Map must be rectangular");
        }
        for (int value : row) {
            if (value < 0) {
                throw std::invalid_argument(
                    "Map cells must be non-negative"
                );
            }
        }
    }
}

Map Map::load(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    std::vector<std::vector<int>> cells;
    std::string line;

    while (std::getline(file, line)) {
        std::stringstream input(line);
        std::vector<int> row;
        int value;
        while (input >> value) {
            row.push_back(value);
        }
        if (!row.empty()) {
            cells.push_back(row);
        }
    }

    return Map(cells);
}

int Map::get_width() const {
    return width_;
}

int Map::get_height() const {
    return height_;
}

bool Map::in_bounds(int x, int y) const {
    return (x >= 0 && x < width_) && (y >= 0 && y < height_);
}

int Map::wall_id(int x, int y) const {
    if (!in_bounds(x, y)) {
        return 1; // за пределами карты возвращает 1 (виртуальную стену)
    }
    return cells_[y][x];
}

bool Map::is_wall(int x, int y) const {
    return wall_id(x, y) > 0;
}
