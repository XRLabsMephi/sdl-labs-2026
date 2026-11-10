#include "wolf/input/PlayerController.hpp"

#include "wolf/world/Map.hpp"
#include "wolf/world/Player.hpp"

#include <algorithm>
#include <cmath>

namespace wolf
{

PlayerController::PlayerController(double moveSpeed, double rotationSpeed)
    : moveSpeed_(moveSpeed)
    , rotationSpeed_(rotationSpeed)
{
}

void PlayerController::update(Player& player, const Map& map, const InputState& input, double deltaSeconds) const
{
    const double elapsed = std::clamp(deltaSeconds, 0.0, 0.1);
    const double forwardAxis = static_cast<double>(input.moveForward) - static_cast<double>(input.moveBackward);
    const double strafeAxis = static_cast<double>(input.strafeRight) - static_cast<double>(input.strafeLeft);

    Vec2 movement = player.direction.normalized() * forwardAxis;
    // Positive camera-plane coordinates are the right side of the rendered view.
    movement += player.cameraPlane.normalized() * strafeAxis;

    if (movement.length() > 1.0)
    {
        movement = movement.normalized();
    }
    move(player, map, movement.x * moveSpeed_ * elapsed, movement.y * moveSpeed_ * elapsed);

    const double turnAxis = static_cast<double>(input.turnRight) - static_cast<double>(input.turnLeft);
    const double angle = turnAxis * rotationSpeed_ * elapsed;
    const double cosine = std::cos(angle);
    const double sine = std::sin(angle);

    const Vec2 oldDirection = player.direction;
    player.direction = {oldDirection.x * cosine - oldDirection.y * sine,
                        oldDirection.x * sine + oldDirection.y * cosine};

    const Vec2 oldPlane = player.cameraPlane;
    player.cameraPlane = {oldPlane.x * cosine - oldPlane.y * sine,
                          oldPlane.x * sine + oldPlane.y * cosine};
}

void PlayerController::move(Player& player, const Map& map, double offsetX, double offsetY) const
{
    const double nextX = player.position.x + offsetX;
    const double nextY = player.position.y + offsetY;

    const auto hitsWall = [&map, this](double x, double y)
    {
        return map.isWall(static_cast<int>(std::floor(x - collisionRadius_)), static_cast<int>(std::floor(y - collisionRadius_)))
            || map.isWall(static_cast<int>(std::floor(x + collisionRadius_)), static_cast<int>(std::floor(y - collisionRadius_)))
            || map.isWall(static_cast<int>(std::floor(x - collisionRadius_)), static_cast<int>(std::floor(y + collisionRadius_)))
            || map.isWall(static_cast<int>(std::floor(x + collisionRadius_)), static_cast<int>(std::floor(y + collisionRadius_)));
    };

    if (!hitsWall(nextX, player.position.y))
    {
        player.position.x = nextX;
    }
    if (!hitsWall(player.position.x, nextY))
    {
        player.position.y = nextY;
    }
}

} // namespace wolf
