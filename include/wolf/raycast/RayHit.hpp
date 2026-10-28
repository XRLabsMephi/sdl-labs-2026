#pragma once

#include <limits>

namespace wolf {

    namespace {

        constexpr double INF = std::numeric_limits<double>::infinity();
        constexpr double minDepth = 1e-6;

    }

    enum class WallSide {
        West,
        East,
        North,
        South,
    };

    [[nodiscard]] constexpr bool isXSide(WallSide side) noexcept {
        return side == WallSide::West || side == WallSide::East;
    }

    struct RayHit {
        bool hit = false;
        double depth = INF;
        int mapX = 0;
        int mapY = 0;
        int wallId = 0;
        WallSide side = WallSide::West;
    };

}
