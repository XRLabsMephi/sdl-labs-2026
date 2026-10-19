//
// Created by Nikita on 17.10.2026.
//
#include "Map.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <utility>

bool world::Map::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);

    if (!file) {
        return false;
    }

    int newWidth;
    int newHeight;

    if (!(file >> newWidth >> newHeight)) {
        return false;
    }

    if (newWidth <= 0 || newHeight <= 0) {
        return false;
    }

    std::vector<std::vector<int>> newCells(
        newHeight,
        std::vector<int>(newWidth)
    );

    for (int y = 0; y < newHeight; ++y) {
        for (int x = 0; x < newWidth; ++x) {
            if (!(file >> newCells[y][x])) {
                return false;
            }

            if (newCells[y][x] < 0) {
                return false;
            }
        }
    }

    core::Vector2 newSpawn{};

    if (!(file >> newSpawn.x >> newSpawn.y)) {
        return false;
    }

    if (!std::isfinite(newSpawn.x) ||
        !std::isfinite(newSpawn.y)) {
        return false;
    }

    if (newSpawn.x < 0 || newSpawn.x >= newWidth ||
        newSpawn.y < 0 || newSpawn.y >= newHeight) {
        return false;
    }

    int spawnX = static_cast<int>(std::floor(newSpawn.x));
    int spawnY = static_cast<int>(std::floor(newSpawn.y));

    if (newCells[spawnY][spawnX] != 0) {
        return false;
    }

    cells = std::move(newCells);
    width = newWidth;
    height = newHeight;
    spawnPosition = newSpawn;

    return true;
}

bool world::Map::canOccupy(core::Vector2 position, float radius) const {
    if (cells.empty()) {
        return false;
    }

    if (!(radius >= 0.0f &&
          position.x - radius >= 0.0f &&
          position.y - radius >= 0.0f &&
          position.x + radius < static_cast<float>(width) &&
          position.y + radius < static_cast<float>(height))) {
        return false;
          }

    int left = static_cast<int>(
        std::floor(position.x - radius)
    );

    int right = static_cast<int>(
        std::floor(position.x + radius)
    );

    int top = static_cast<int>(
        std::floor(position.y - radius)
    );

    int bottom = static_cast<int>(
        std::floor(position.y + radius)
    );

    for (int y = top; y <= bottom; ++y) {
        for (int x = left; x <= right; ++x) {
            if (!isWall(x, y)) {
                continue;
            }

            float nearestX = std::clamp(
                static_cast<float>(position.x),
                static_cast<float>(x),
                static_cast<float>(x + 1)
            );

            float nearestY = std::clamp(
                static_cast<float>(position.y),
                static_cast<float>(y),
                static_cast<float>(y + 1)
            );

            float dx = position.x - nearestX;
            float dy = position.y - nearestY;

            if (dx * dx + dy * dy <= radius * radius) {
                return false;
            }
        }
    }

    return true;
}