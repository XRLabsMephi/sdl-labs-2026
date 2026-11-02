#pragma once
#include "wolf/core/Vec2.hpp"

struct Sprite {
    Vec2 position;
    int textureId = 0;
    double scale = 1.0;
};