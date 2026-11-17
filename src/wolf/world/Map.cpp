#include "wolf/world/Map.hpp"
#include <fstream>
#include <cstddef>

Map::Map() : width(0), height(0) {}

bool Map::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    int newWidth;
    int newHeight;
    if (!(file >> newWidth >> newHeight)) return false;

    constexpr int maxSize = 1024;
    if (newWidth < 3 || newHeight < 3 || newWidth > maxSize || newHeight > maxSize) return false;

    std::vector<int> newCells(static_cast<std::size_t>(newWidth) * static_cast<std::size_t>(newHeight));

    for (int y = 0; y < newHeight; ++y) {
        for (int x = 0; x < newWidth; ++x) {
            int value;
            if (!(file >> value)) return false;
            if (value < 0) return false;

            bool boundary = x == 0 || y == 0 || x == newWidth - 1 || y == newHeight - 1;
            if (boundary && value == 0) return false;

            newCells[y * newWidth + x] = value;
        }
    }

    file >> std::ws;
    if (!file.eof() || file.bad()) return false;

    cells.swap(newCells);
    width = newWidth;
    height = newHeight;
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
