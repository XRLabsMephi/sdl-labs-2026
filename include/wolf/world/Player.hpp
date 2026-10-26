#pragma once

#include "wolf/core/Vec2.hpp"

namespace wolf
{

struct Player
{
    Vec2 position{1.5, 1.5};
    Vec2 direction{1.0, 0.0};
    // Points to the right of the camera; length = tan(60 degrees / 2).
    Vec2 cameraPlane{0.0, 0.5773502691896257};
};

}
