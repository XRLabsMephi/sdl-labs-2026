#pragma once

#include "wolf/core/Vec2.hpp"
#include "wolf/world/Player.hpp"

class Raycaster {
public:
    Vec2 rayDirection(const Player& player, double cameraX) const;
};