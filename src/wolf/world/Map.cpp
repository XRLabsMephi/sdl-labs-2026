#include "../../../include/wolf/world/Map.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <utility>

bool world::Map::loadFromFile(const std::string &filename) {
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

    // Parse into temporary storage to preserve the current map if validation fails.
    std::vector<std::vector<int8_t> > newCells(
        newHeight,
        std::vector<int8_t>(newWidth)
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
    // The spawn point must be inside the map and in an empty cell.
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

    // Commit the new map only after all input has been validated.
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

    // The player's entire collision circle must remain within the map.
    if (!(radius >= 0.0f &&
          position.x - radius >= 0.0f &&
          position.y - radius >= 0.0f &&
          position.x + radius < static_cast<float>(width) &&
          position.y + radius < static_cast<float>(height))) {
        return false;
    }

    // Find the grid cells covered by the circle's bounding box.
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

    for (float y = top; y <= bottom; y += 1.0) {
        for (float x = left; x <= right; x += 1.0) {
            if (!isWall(x, y)) {
                continue;
            }

            float nearestX = std::clamp(
                position.x,
                x,
                x + 1
            );

            float nearestY = std::clamp(
                position.y,
                y,
                y + 1
            );

            float dx = position.x - nearestX;
            float dy = position.y - nearestY;

            // Touching or overlapping a wall blocks the position.
            // Compare squared distances to avoid computing a square root.
            if (dx * dx + dy * dy <= radius * radius) {
                return false;
            }
        }
    }

    return true;
}
