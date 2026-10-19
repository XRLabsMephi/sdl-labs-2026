#pragma once
#include <vector>
#include "../core/Vec2.h"
#include <string>
namespace world {
    class Map {
    private:
        std::vector<std::vector<int>> cells;

        int width = 0;
        int height = 0;

        core::Vector2 spawnPosition;

    public:
        bool loadFromFile(const std::string& filename);

        int getWidth() const {
            return width;
        }

        int getHeight() const {
            return height;
        }

        bool isInside(int x, int y) const {
            return x >= 0 && x < width &&
                   y >= 0 && y < height;
        }

        int getCell(int x, int y) const {
            if (!isInside(x, y)) {
                return -1;
            }

            return cells[y][x];
        }

        bool isWall(int x, int y) const {
            return getCell(x, y) != 0;
        }

        bool canOccupy(core::Vector2 position, float radius) const;

        core::Vector2 getSpawnPosition() const {
            return spawnPosition;
        }
    };
}