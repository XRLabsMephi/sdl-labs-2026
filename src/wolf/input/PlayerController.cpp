#include "wolf/input/PlayerController.hpp"

#include <cmath>

namespace wolf
{
bool PlayerController::canStand(const Map& map, double x, double y) const
{
    constexpr double radius = 0.20;

    if (map.isWall(static_cast<int>(std::floor(x - radius)), static_cast<int>(std::floor(y - radius))))
    {
        return false;
    }

    if (map.isWall(static_cast<int>(std::floor(x + radius)), static_cast<int>(std::floor(y - radius))))
    {
        return false;
    }

    if (map.isWall(static_cast<int>(std::floor(x - radius)), static_cast<int>(std::floor(y + radius))))
    {
        return false;
    }

    if (map.isWall(static_cast<int>(std::floor(x + radius)), static_cast<int>(std::floor(y + radius))))
    {
        return false;
    }

    return true;
}

void PlayerController::rotate(Player& player, double angle) const
{
    const double c = std::cos(angle), s = std::sin(angle);

    const double oldDirX = player.direction.x;
    player.direction.x = player.direction.x * c - player.direction.y * s;
    player.direction.y = oldDirX * s + player.direction.y * c;

    const double oldPlaneX = player.cameraPlane.x;
    player.cameraPlane.x = player.cameraPlane.x * c - player.cameraPlane.y * s;
    player.cameraPlane.y = oldPlaneX * s + player.cameraPlane.y * c;
}

void PlayerController::update(Player& player, const Map& map, const InputState& input, double deltaTime) const
{
    constexpr double moveSpeed = 2.5;
    constexpr double rotationSpeed = 1.8;

    double forward = 0.0, strafe = 0.0;

    if (input.forward)
    {
        forward += moveSpeed * deltaTime;
    }

    if (input.backward)
    {
        forward -= moveSpeed * deltaTime;
    }

    if (input.strafeLeft)
    {
        strafe -= moveSpeed * deltaTime;
    }

    if (input.strafeRight)
    {
        strafe += moveSpeed * deltaTime;
    }

    if (forward != 0.0 && strafe != 0.0)
    {
        forward *= 0.70710678118;
        strafe *= 0.70710678118;
    }

    if (input.turnLeft)
    {
        rotate(player, -rotationSpeed * deltaTime);
    }

    if (input.turnRight)
    {
        rotate(player, rotationSpeed * deltaTime);
    }

    const Vec2 right{-player.direction.y, player.direction.x};
    const Vec2 movement = player.direction * forward + right * strafe;

    if (canStand(map, player.position.x + movement.x, player.position.y))
    {
        player.position.x += movement.x;
    }

    if (canStand(map, player.position.x, player.position.y + movement.y))
    {
        player.position.y += movement.y;
    }
}
}
