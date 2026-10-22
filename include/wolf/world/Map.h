#pragma once
#include <vector>
#include "../core/Vec2.h"
#include <string>
#include <cstdint>
namespace world {
    class Map {
    private:
        // Indexed as cells[y][x]: 0 means empty space, positive values identify wall types.
        std::vector<std::vector<int8_t>> cells;

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
            // Return a nonzero sentinel so out-of-bounds cells are treated as walls.
            if (!isInside(x, y)) {
                return -1;
            }

            return cells[y][x];
        }

        bool isWall(int x, int y) const {
            return getCell(x, y) != 0;
        }

        bool canOccupy(core::Vector2 position, double radius) const;

        core::Vector2 getSpawnPosition() const {
            return spawnPosition;
        }
    };
}