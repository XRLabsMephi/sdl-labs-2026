#include "wolf/raycast/Raycaster.hpp"
#include "wolf/world/Player.hpp"


Vec2 Raycaster::rayDirection(const Player& player, double cameraX) const {
    return player.direction + player.cameraPlane * cameraX;
}