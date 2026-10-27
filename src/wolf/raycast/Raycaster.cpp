#include "wolf/raycast/Raycaster.hpp"

#include "wolf/world/Map.hpp"
#include "wolf/world/Player.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <optional>

namespace wolf {

    namespace {

        constexpr double INF = std::numeric_limits<double>::infinity();
        constexpr double minDepth = 1e-6;

        int toRow(double row, int screenHeight) {
            return static_cast<int>(std::clamp(row, 0.0, static_cast<double>(screenHeight)));
        }

    }

    double cameraXForColumn(int column, int screenWidth) noexcept {
        if (screenWidth <= 0) {
            return 0.0;
        }
        return 2.0 * column / static_cast<double>(screenWidth) - 1.0;
    }

    Vec2 rayDirection(const Player& player, double cameraX) noexcept {
        return player.direction + player.cameraPlane * cameraX;
    }

    RayHit castRay(const Map& map, const Player& player, double cameraX) noexcept {
        RayHit result;
        const Vec2 position = player.position;
        const Vec2 rayDir = rayDirection(player, cameraX);
        if (!(isFinite(player.position) && isFinite(rayDir))) {
            return result;
        }
        if (position.x < 0.0 || position.x >= map.width()) {
            return result;
        }
        if (position.y < 0.0 || position.y >= map.height()) {
            return result;
        }
        int mapX = static_cast<int>(std::floor(position.x));
        int mapY = static_cast<int>(std::floor(position.y));
        if (!map.isEmpty(mapX, mapY)) {
            return result;
        }
        if (!std::isfinite(player.direction.length()) || player.direction.length() < minVectorLength) {
            return result;
        }
        const double forwardPerT = dot(rayDir, player.direction) / player.direction.length();
        if (forwardPerT <= 0.0) {
            return result;
        }
        const double deltaDistX = rayDir.x == 0.0 ? INF : std::abs(1.0 / rayDir.x);
        const double deltaDistY = rayDir.y == 0.0 ? INF : std::abs(1.0 / rayDir.y);
        int stepX = 1, stepY = 1;
        double sideDistX = (mapX + 1.0 - position.x) * deltaDistX;
        double sideDistY = (mapY + 1.0 - position.y) * deltaDistY;
        if (rayDir.x < 0.0) {
            stepX = -1;
            sideDistX = (position.x - mapX) * deltaDistX;
        }
        if (rayDir.y < 0.0) {
            stepY = -1;
            sideDistY = (position.y - mapY) * deltaDistY;
        }
        while (true) {
            double t = 0.0;
            WallSide side = WallSide::West;
            if (sideDistX < sideDistY) {
                t = sideDistX;
                sideDistX += deltaDistX;
                mapX += stepX;
                side = stepX > 0 ? WallSide::West : WallSide::East;
            } else {
                t = sideDistY;
                sideDistY += deltaDistY;
                mapY += stepY;
                side = stepY > 0 ? WallSide::North : WallSide::South;
            }
            const std::optional<int> cell = map.cellAt(mapX, mapY);
            if (!cell) {
                return result;
            }
            if (*cell > 0) {
                result.hit = true;
                result.depth = t * forwardPerT;
                result.mapX = mapX;
                result.mapY = mapY;
                result.wallId = *cell;
                result.side = side;
                return result;
            }
        }
    }

    double focalLength(const Player& player, int screenWidth) noexcept {
        if (screenWidth <= 0) {
            return 0.0;
        }
        if (!(std::isfinite(player.direction.length()) && std::isfinite(player.cameraPlane.length()))) {
            return 0.0;
        }
        if (player.cameraPlane.length() < minVectorLength) {
            return 0.0;
        }
        return 0.5 * screenWidth * player.direction.length() / player.cameraPlane.length();
    }

    WallSlice projectWall(const RayHit& hit, double focal, int screenHeight) noexcept {
        WallSlice slice;
        slice.hit = hit;
        if (!hit.hit || std::isnan(hit.depth)) {
            return slice;
        }
        if (!std::isfinite(focal) || focal <= 0.0) {
            return slice;
        }
        if (screenHeight <= 0) {
            return slice;
        }
        const double depth = std::max(hit.depth, minDepth);
        slice.height = focal / depth;
        const double horizon = 0.5 * screenHeight;
        const double top = horizon - 0.5 * slice.height;
        const double bottom = horizon + 0.5 * slice.height;
        slice.drawStart = toRow(std::ceil(top - 0.5), screenHeight);
        slice.drawEnd = toRow(std::ceil(bottom - 0.5), screenHeight);
        return slice;
    }

    void Raycaster::castFrame(
            const Map& map,
            const Player& player,
            int screenWidth,
            int screenHeight) {
        columns_.clear();
        depthBuffer_.clear();
        if (screenWidth <= 0 || screenHeight <= 0) {
            return;
        }
        columns_.reserve(static_cast<std::size_t>(screenWidth));
        depthBuffer_.reserve(static_cast<std::size_t>(screenWidth));
        for (int x = 0; x < screenWidth; ++x) {
            const RayHit hit = castRay(map, player, cameraXForColumn(x, screenWidth));
            columns_.push_back(projectWall(hit, focalLength(player, screenWidth), screenHeight));
            depthBuffer_.push_back(hit.depth);
        }
    }

    const std::vector<WallSlice>& Raycaster::columns() const noexcept {
        return columns_;
    }

    const std::vector<double>& Raycaster::depthBuffer() const noexcept {
        return depthBuffer_;
    }

}
