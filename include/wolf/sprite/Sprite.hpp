#pragma once
#include "wolf/core/Vec2.hpp"
#include <cstdint>

namespace wolf::sprite {
    struct Sprite {
        core::Vec2 position;
        int textureId = 0;
        double scale = 1.0;
    };
}