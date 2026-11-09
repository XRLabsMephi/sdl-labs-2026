#include "wolf/input/PlayerController.hpp"

#include <algorithm>
#include <cmath>

namespace wolf {

bool PlayerController::can_move(const Map& map, Vec2 position) const {
    int left = static_cast<int>(
        std::floor(position.x - half_size_)
    );

    int right = static_cast<int>(
        std::floor(position.x + half_size_)
    );

    int top = static_cast<int>(
        std::floor(position.y - half_size_)
    );

    int bottom = static_cast<int>(
        std::floor(position.y + half_size_)
    );

    return !map.is_wall(left, top) &&
           !map.is_wall(right, top) &&
           !map.is_wall(left, bottom) &&
           !map.is_wall(right, bottom);
}

void PlayerController::update(Player& player, const Map& map, const InputState& input,double dt) const {
    if (dt <= 0.0) {
        return;
    }

    // 1. Поворот камеры.
    int turn = static_cast<int>(input.turnRight) -
               static_cast<int>(input.turnLeft);
    double angle = turn * rotation_speed_ * dt;
    double cos_angle = std::cos(angle);
    double sin_angle = std::sin(angle);
    Vec2 direction = player.direction;
    Vec2 plane = player.cameraPlane;

    player.direction = {
        direction.x * cos_angle - direction.y * sin_angle,
        direction.x * sin_angle + direction.y * cos_angle
    };

    player.cameraPlane = {
        plane.x * cos_angle - plane.y * sin_angle,
        plane.x * sin_angle + plane.y * cos_angle
    };

    // 2. Направление перемещения.
    int forward = static_cast<int>(input.moveForward) -
                  static_cast<int>(input.moveBackward);
    int side = static_cast<int>(input.moveRight) -
               static_cast<int>(input.moveLeft);

    Vec2 move{
        player.direction.x * forward -
            player.direction.y * side,

        player.direction.y * forward +
            player.direction.x * side
    };

    // Normalize before applying speed: diagonal movement is not faster.
    const double length = std::hypot(move.x, move.y);
    Vec2 target_velocity{0.0, 0.0};
    if (length > 0.0) {
        const double speed = move_speed_ * (input.sprint ? 1.8 : 1.0);
        target_velocity = {
            move.x / length * speed,
            move.y / length * speed
        };
    }

    // Frame-rate independent acceleration and friction.
    const double response = input.drift ? 1.8 : (length > 0.0 ? 14.0 : 18.0);
    const double blend = 1.0 - std::exp(-response * dt);
    player.velocity.x += (target_velocity.x - player.velocity.x) * blend;
    player.velocity.y += (target_velocity.y - player.velocity.y) * blend;

    if (length == 0.0 && std::hypot(player.velocity.x, player.velocity.y) < 0.01) {
        player.velocity = {0.0, 0.0};
    }

    Vec2 displacement{player.velocity.x * dt, player.velocity.y * dt};
    const double distance = std::hypot(displacement.x, displacement.y);
    if (distance == 0.0) {
        return;
    }

    // Substep the actual (possibly sprinting) displacement.
    const int steps = std::max(
        1, static_cast<int>(std::ceil(distance / half_size_))
    );
    displacement.x /= steps;
    displacement.y /= steps;

    // Slide along walls by resolving X and Y separately.
    for (int i = 0; i < steps; ++i) {
        Vec2 next = player.position;
        next.x += displacement.x;
        if (can_move(map, next)) {
            player.position.x = next.x;
        } else {
            player.velocity.x = 0.0;
        }

        next = player.position;
        next.y += displacement.y;
        if (can_move(map, next)) {
            player.position.y = next.y;
        } else {
            player.velocity.y = 0.0;
        }
    }
}

}
