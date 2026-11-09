#pragma once

#include "wolf/core/Vec2.hpp"

namespace wolf
{
struct RayHit
{
    bool hit = false;
    double depth = 0.0;
    Vec2 position;
    int mapX = 0, mapY = 0;
    int wallId = 0;
    int side = 0;
};
}
