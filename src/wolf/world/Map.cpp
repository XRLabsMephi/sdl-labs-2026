#include <world/Map.h>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <limits>
#include <utility>
/*
Example of file for map
8 8
1 1 1 1 1 1 1 1
1 0 0 0 0 0 0 1
1 0 2 2 0 0 0 1
1 0 0 2 0 3 0 1
1 0 0 0 0 3 0 1
1 0 4 0 0 0 0 1
1 0 0 0 0 0 0 1
1 1 1 1 1 1 1 1
1.5 1.5
 */
bool world::Map::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);

    if (!file) {
        return false;
    }

    int newWidth = 0;
    int newHeight = 0;

    if (!(file >> newWidth >> newHeight)) {
        return false;
    }

    if (newWidth <= 0 || newHeight <= 0) {
        return false;
    }

    // Preserve the current map until all input has been validated.
    std::vector<std::vector<std::int8_t>> newCells(
        newHeight,
        std::vector<std::int8_t>(newWidth)
    );

    for (int y = 0; y < newHeight; ++y) {
        for (int x = 0; x < newWidth; ++x) {
            int cell = 0;

            // Read as an integer because int8_t uses character extraction.
            if (!(file >> cell)) {
                return false;
            }

            if (cell < 0 ||
                cell > std::numeric_limits<std::int8_t>::max()) {
                return false;
            }

            newCells[y][x] = static_cast<std::int8_t>(cell);
        }
    }

    // Require enclosing walls so rays hit a valid wall before leaving the map.
    for (int x = 0; x < newWidth; ++x) {
        if (newCells[0][x] == 0 ||
            newCells[newHeight - 1][x] == 0) {
            return false;
        }
    }

    for (int y = 0; y < newHeight; ++y) {
        if (newCells[y][0] == 0 ||
            newCells[y][newWidth - 1] == 0) {
            return false;
        }
    }

    core::Vector2 newSpawn{};

    if (!(file >> newSpawn.x >> newSpawn.y)) {
        return false;
    }

    // Validate coordinates before converting them to cell indices.
    if (!(newSpawn.x >= 0.0 &&
          newSpawn.x < newWidth &&
          newSpawn.y >= 0.0 &&
          newSpawn.y < newHeight)) {
        return false;
    }

    const int spawnX = static_cast<int>(std::floor(newSpawn.x));
    const int spawnY = static_cast<int>(std::floor(newSpawn.y));

    if (newCells[spawnY][spawnX] != 0) {
        return false;
    }

    // Commit the validated map.
    cells = std::move(newCells);
    width = newWidth;
    height = newHeight;
    spawnPosition = newSpawn;

    return true;
}

bool world::Map::canOccupy(
    core::Vector2 position,
    double radius
) const {
    if (cells.empty()) {
        return false;
    }

    // Reject invalid values and circles extending beyond the map.
    if (!(radius >= 0.0 &&
          position.x - radius >= 0.0 &&
          position.y - radius >= 0.0 &&
          position.x + radius < width &&
          position.y + radius < height)) {
        return false;
    }

    const int left = static_cast<int>(
        std::floor(position.x - radius)
    );

    const int right = static_cast<int>(
        std::floor(position.x + radius)
    );

    const int top = static_cast<int>(
        std::floor(position.y - radius)
    );

    const int bottom = static_cast<int>(
        std::floor(position.y + radius)
    );

    // Check wall cells covered by the circle's bounding box.
    for (int y = top; y <= bottom; ++y) {
        for (int x = left; x <= right; ++x) {
            if (!isWall(x, y)) {
                continue;
            }

            const double nearestX = std::clamp(
                position.x,
                static_cast<double>(x),
                static_cast<double>(x + 1)
            );

            const double nearestY = std::clamp(
                position.y,
                static_cast<double>(y),
                static_cast<double>(y + 1)
            );

            const double dx = position.x - nearestX;
            const double dy = position.y - nearestY;

            // Touching a wall also blocks the position.
            if (dx * dx + dy * dy <= radius * radius) {
                return false;
            }
        }
    }

    return true;
}