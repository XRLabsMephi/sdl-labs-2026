#include "wolf/input/PlayerController.hpp"

#include "wolf/input/InputState.hpp"
#include "wolf/world/Map.hpp"
#include "wolf/world/Player.hpp"

#include <algorithm>
#include <cmath>

namespace wolf {

    namespace {

        constexpr double moveSpeed = 3.0;
        constexpr double turnSpeed = 1.8;
        constexpr double playerHalfSize = 0.2;
        constexpr double maxStepDistance = 0.1;
        constexpr double maxDeltaTime = 1.0;

        bool withinBounds(const Map& map, Vec2 position) {
            if (!std::isfinite(position.x) || !std::isfinite(position.y)) {
                return false;
            }
            if (position.x - playerHalfSize < 0.0 || position.x + playerHalfSize >= map.width()) {
                return false;
            }
            if (position.y - playerHalfSize < 0.0 || position.y + playerHalfSize >= map.height()) {
                return false;
            }
            return true;
        }

        bool canOccupy(const Map& map, Vec2 position) {
            if (!withinBounds(map, position)) {
                return false;
            }
            const int minX = static_cast<int>(std::floor(position.x - playerHalfSize));
            const int maxX = static_cast<int>(std::floor(position.x + playerHalfSize));
            const int minY = static_cast<int>(std::floor(position.y - playerHalfSize));
            const int maxY = static_cast<int>(std::floor(position.y + playerHalfSize));
            for (int y = minY; y <= maxY; ++y) {
                for (int x = minX; x <= maxX; ++x) {
                    if (!map.isEmpty(x, y)) {
                        return false;
                    }
                }
            }
            return true;
        }

        Vec2 rotate(Vec2 value, double cosine, double sine) {
            return {
                value.x * cosine - value.y * sine,
                value.x * sine + value.y * cosine
            };
        }

    }

    void PlayerController::update(
            Player& player,
            const Map& map,
            const InputState& input,
            double dt) noexcept {
        if (input.quit || !std::isfinite(dt) || dt <= 0.0 || !canOccupy(map, player.position)) {
            return;
        }
        const double directionLength = std::hypot(player.direction.x, player.direction.y);
        const double planeLength = std::hypot(player.cameraPlane.x, player.cameraPlane.y);
        if (!std::isfinite(directionLength) || directionLength < 1e-12 ||
            !std::isfinite(planeLength) || planeLength < 1e-12) {
            return;
        }
        dt = std::min(dt, maxDeltaTime);
        const int turn = static_cast<int>(input.turnRight) - static_cast<int>(input.turnLeft);
        const double angle = turn * turnSpeed * dt;
        player.direction = rotate(player.direction, std::cos(angle), std::sin(angle));
        player.cameraPlane = rotate(player.cameraPlane, std::cos(angle), std::sin(angle));
        const int forward = static_cast<int>(input.moveForward) -
                            static_cast<int>(input.moveBackward);
        const int strafe = static_cast<int>(input.strafeRight) -
                           static_cast<int>(input.strafeLeft);
        const Vec2 facing = player.direction * (1.0 / directionLength);
        const Vec2 right{-facing.y, facing.x};
        Vec2 movement = facing * forward + right * strafe;
        const double length = std::hypot(movement.x, movement.y);
        if (length == 0.0) {return;}
        movement = movement * (moveSpeed * dt / length);
        const int steps = static_cast<int>(std::ceil(moveSpeed * dt / maxStepDistance));
        const Vec2 step = movement * (1.0 / steps);
        for (int i = 0; i < steps; ++i) {
            if (canOccupy(map, {player.position.x + step.x, player.position.y})) {
                player.position.x += step.x;
            }
            if (canOccupy(map, {player.position.x, player.position.y + step.y})) {
                player.position.y += step.y;
            }
        }
    }

}
