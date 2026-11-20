#pragma once

#include "wolf/core/Vec2.hpp"

namespace sprite {
    struct Sprite {
        core::Vec2 position;
        int textureId = 0;
        // World-space height. The bottom rests on the floor; PNG aspect ratio is preserved.
        double scale = 1.0;
    };
}
