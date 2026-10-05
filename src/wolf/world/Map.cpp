#include "wolf/world/Map.hpp"

#include <fstream>
#include <stdexcept>

namespace world {
    Map::Map(std::vector<std::vector<int>> cells) : cells_(std::move(cells)) {}

    Map Map::loadFromFile(const std::string &path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open map file: " + path);
        }

        std::vector<std::vector<int>> cells;
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) {
                continue;
            }
            std::vector<int> row;
            row.reserve(line.size());
            for (char ch : line) {
                if (ch == '.' || ch == '0') {
                    row.push_back(0);
                } else if (ch >= '1' && ch <= '9') {
                    row.push_back(ch - '0');
                } else if (ch == '#') {
                    row.push_back(1);
                } else {
                    row.push_back(0);
                }
            }
            cells.push_back(std::move(row));
        }

        return Map(std::move(cells));
    }

    Map Map::defaultMap() {
        std::vector<std::vector<int>> cells = {
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 2, 2, 2, 0, 0, 0, 3, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        };
        return Map(std::move(cells));
    }

    int Map::width() const noexcept {
        if (cells_.empty()) {
            return 0;
        }
        return (int)cells_[0].size();
    }

    int Map::height() const noexcept {
        return (int)cells_.size();
    }

    bool Map::isInBounds(int x, int y) const noexcept {
        return x >= 0 && y >= 0 && y < height() && x < width();
    }

    int Map::cellAt(int x, int y) const noexcept {
        if (!isInBounds(x, y)) {
            return 0;
        }
        return cells_[y][x];
    }

    bool Map::isSolid(int x, int y) const noexcept {
        if (!isInBounds(x, y)) {
            return true;
        }
        return cells_[y][x] != 0;
    }
}
