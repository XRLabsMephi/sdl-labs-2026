#pragma once

#include "wolf/core/Vec2.hpp"

namespace wolf
{

struct Sprite
{
    Vec2 position;
    int textureId;
    double scale = 1.0;
};

} // namespace wolf
