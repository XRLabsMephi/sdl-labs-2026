#include "wolf/world/Map.hpp"

#include <iostream>
#include <fstream>
#include <stdexcept>

namespace world {
    bool Map::areMapCellsValid(const std::vector<std::vector<int>> &cells) {
        // Is recatangle
        for (size_t i = 1; i < cells.size(); i++) {
            if (cells[i].size() != cells[i - 1].size()) {
                return false;
            }
        }
        return true;
    }


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

        if (!areMapCellsValid(cells)) {
            throw std::runtime_error("Map in \"" + path + "\" is not valid");
        }

        return Map(std::move(cells));
    }

    Map Map::initMap() {
        std::vector<std::vector<int>> cells = {
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 2, 2, 2, 0, 0, 0, 3, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0},
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

    Map Map::initMap(const std::string &filePath) {
        try {
            Map map = loadFromFile(filePath);
            return map;
        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
            return initMap();
        }
    }

    size_t Map::width() const noexcept {
        if (cells_.empty()) {
            return 0;
        }
        return cells_[0].size();
    }

    size_t Map::height() const noexcept {
        return cells_.size();
    }

    bool Map::isInBounds(int x, int y) const noexcept {
        return x >= 0 && y >= 0 && y < (int) height() && x < (int) width();
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
