#pragma once

#include "wolf/core/Vec2.hpp"
#include "wolf/raycast/RayHit.hpp"

#include <vector>

namespace wolf {

    class Map;
    struct Player;

    struct WallSlice {
        RayHit hit;
        double height = 0.0;
        int drawStart = 0;
        int drawEnd = 0;
    };

    [[nodiscard]] double cameraXForColumn(int column, int screenWidth) noexcept;

    [[nodiscard]] Vec2 rayDirection(const Player& player, double cameraX) noexcept;

    [[nodiscard]] RayHit castRay(const Map& map, const Player& player, double cameraX) noexcept;

    [[nodiscard]] double focalLength(const Player& player, int screenWidth) noexcept;

    [[nodiscard]] double projectedSize(double depth, double focal) noexcept;

    [[nodiscard]] WallSlice projectWall(const RayHit& hit, double focal, int screenHeight) noexcept;

    [[nodiscard]] int wallTextureX(double wallU, int textureWidth) noexcept;

    [[nodiscard]] int wallTextureY(
            const WallSlice& slice,
            int screenY,
            int screenHeight,
            int textureHeight) noexcept;

    struct CameraPoint {
        double depth = 0.0;
        double cameraX = 0.0;
    };

    [[nodiscard]] CameraPoint toCameraSpace(const Player& player, Vec2 point) noexcept;

    [[nodiscard]] double screenXForCameraX(double cameraX, int screenWidth) noexcept;

    class Raycaster {
        public:
            void castFrame(const Map& map, const Player& player, int screenWidth, int screenHeight);

            [[nodiscard]] const std::vector<WallSlice>& columns() const noexcept;

            [[nodiscard]] const std::vector<double>& depthBuffer() const noexcept;

        private:
            std::vector<WallSlice> columns_;
            std::vector<double> depthBuffer_;
    };

}
