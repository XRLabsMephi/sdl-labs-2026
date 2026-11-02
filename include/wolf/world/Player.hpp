#pragma once

#include <numbers>
#include <cmath>

#include "wolf/core/Vec2.hpp"

inline const double tan30 = std::tan(std::numbers::pi / 6);

struct Player {
    Vec2 position{1.5, 1.5};
    Vec2 direction{1.0, 0.0};
    Vec2 cameraPlane{0.0,tan30};
};