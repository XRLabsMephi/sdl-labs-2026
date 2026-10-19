//
// Created by Nikita on 19.10.2026.
//

#include "wolf/world/Map.hpp"
#include <fstream>

Map::Map() : width(0), height(0) {}

bool Map::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    file >> width >> height;
    if (width <= 0 || height <= 0) {
        return false;
    }
    cells.resize(width * height);
    for (int i = 0; i < width * height; i++) {
        if (!(file >> cells[i])) {
            return false;
        }
    }
    return true;
}

int Map::getWidth() const {
    return width;
}

int Map::getHeight() const {
    return height;
}

bool Map::isInside(int x, int y) const {
    return x >= 0 && x < width && y >= 0 && y < height;
}

int Map::getCell(int x, int y) const {
    if (!isInside(x, y)) {
        return -1;
    }
    return cells[y * width + x];
}

bool Map::isWall(int x, int y) const {
    if (!isInside(x, y)) {
        return true;
    }
    return getCell(x, y) != 0;
}
