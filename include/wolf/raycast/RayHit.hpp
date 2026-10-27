#pragma once

#include <limits>

namespace wolf {

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
        double depth = std::numeric_limits<double>::infinity();
        int mapX = 0;
        int mapY = 0;
        int wallId = 0;
        WallSide side = WallSide::West;
    };

}
