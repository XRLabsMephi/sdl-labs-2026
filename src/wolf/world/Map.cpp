#include "wolf/world/Map.hpp"

namespace wolf::world {
    Map::Map() {
        width = 8;
        height = 8;
        // 0 - пустое, 1-3 типы стен
        grid = {
            1, 1, 1, 1, 1, 1, 1, 1,
            1, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 2, 0, 0, 3, 0, 1,
            1, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 0, 2, 0, 0, 0, 1,
            1, 0, 0, 0, 0, 0, 0, 1,
            1, 1, 1, 1, 1, 1, 1, 1
        };
    }

    int Map::getCell(int x, int y) const {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            return grid[y * width + x];
        }
        return 1;
    }

    int Map::getWidth() const {
        return width;
    }
    int Map::getHeight() const {
        return height;
    }
}