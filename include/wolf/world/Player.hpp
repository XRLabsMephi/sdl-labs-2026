#pragma once

#include <numbers>
#include "wolf/core/Vec2.hpp"

namespace wolf {

    inline constexpr Vec2 playerStartPosition{1.5, 1.5};
    inline constexpr Vec2 playerStartDirection{1.0, 0.0};
    inline constexpr double cameraPlaneLength = std::numbers::inv_sqrt3;

    [[nodiscard]] constexpr Vec2 cameraPlaneFor(Vec2 direction) noexcept {
        return Vec2{-direction.y, direction.x} * cameraPlaneLength;
    }

    struct Player {
        Vec2 position = playerStartPosition;
        Vec2 direction = playerStartDirection;
        Vec2 cameraPlane = cameraPlaneFor(playerStartDirection);
    };

}
