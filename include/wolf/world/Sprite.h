#pragma once

#include <core/Vec2.h>

namespace world {
    struct Sprite {
        core::Vector2 position;
        int textureId = 0;
        double scale = 1.0;
    };
}
