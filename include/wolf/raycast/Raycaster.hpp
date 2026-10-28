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

    [[nodiscard]] WallSlice projectWall(const RayHit& hit, double focal, int screenHeight) noexcept;

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
