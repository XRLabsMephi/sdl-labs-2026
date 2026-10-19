#pragma once

#include "wolf/core/Vec2.hpp"
#include "wolf/world/Player.hpp"
#include "wolf/raycast/RayHit.hpp"

class Map;

class Raycaster {
public:
    Vec2 rayDirection(const Player& player, double cameraX) const;
    RayHit castRay(const Map& map, const Player& player, double cameraX) const;
};