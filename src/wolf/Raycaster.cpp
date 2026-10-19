#include "wolf/raycast/Raycaster.hpp"
#include "wolf/world/Player.hpp"
#include <cmath>
#include <limits>

Vec2 Raycaster::rayDirection(const Player &player, double cameraX) const {
    return player.direction + player.cameraPlane * cameraX;
}

RayHit Raycaster::castRay(const Map &map, const Player &player, double cameraX) const {
    Vec2 direction = rayDirection(player, cameraX);
    int mapX = static_cast<int>(std::floor(player.position.x));
    int mapY = static_cast<int>(std::floor(player.position.y));
    double infinity = std::numeric_limits<double>::infinity();
    double deltaDistX=0.0;
    double deltaDistY=0.0;
    if (direction.x == 0.0) deltaDistX = infinity;
    else deltaDistX = std::abs(1.0 / direction.x);
    if (direction.y == 0.0) deltaDistY = infinity;
    else deltaDistY = std::abs(1.0 / direction.y);
    int stepX;
    int stepY;
    if (direction.x < 0.0) stepX = -1;
    else stepX = 1;
    if (direction.y < 0.0) stepY = -1;
    else stepY = 1;
    return {};
}
