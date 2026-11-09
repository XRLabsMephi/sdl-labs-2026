#include "wolf/input/PlayerController.hpp"

#include <algorithm>
#include <cmath>

namespace wolf {

bool PlayerController::can_move(const Map& map, Vec2 position) const {
    if (!std::isfinite(position.x) || !std::isfinite(position.y)) {
        return false;
    }

    if (position.x - half_size_ < 0.0 ||
        position.y - half_size_ < 0.0 ||
        position.x + half_size_ >= map.get_width() ||
        position.y + half_size_ >= map.get_height()) {
        return false;
        }

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

    player.dashCooldown = std::max(0.0, player.dashCooldown - dt);

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

    // Sustained sprint rewards continuous movement after three seconds.
    player.sprintTime = input.sprint && length > 0.0
        ? player.sprintTime + dt
        : 0.0;

    Vec2 target_velocity{0.0, 0.0};
    if (length > 0.0) {
        const double buildup = std::clamp(
            (player.sprintTime - 3.0) / 1.5, 0.0, 1.0
        );
        const double multiplier = input.sprint ? 1.8 + 0.7 * buildup : 1.0;
        const double speed = move_speed_ * multiplier;
        target_velocity = {
            move.x / length * speed,
            move.y / length * speed
        };
    }

    // Dash has a fixed 150 ms duration, independent of frame rate.
    // Store its direction once so turning doesn't bend the dash.
    constexpr double dash_speed = 14.0;
    if (input.dash && player.dashCooldown <= 0.0 && player.dashRemaining <= 0.0) {
        const Vec2 direction = length > 0.0 ? move : player.direction;
        const double magnitude = std::hypot(direction.x, direction.y);
        if (magnitude > 0.0) {
            player.dashDirection = {direction.x / magnitude, direction.y / magnitude};
            player.dashRemaining = 0.15;
            player.dashCooldown = 0.85;
            player.velocity = {
                player.dashDirection.x * dash_speed,
                player.dashDirection.y * dash_speed
            };
        }
    }

    // The same collision solver handles both regular movement and dashes.
    auto move_with_collisions = [&](Vec2 displacement) {
        const double distance = std::hypot(displacement.x, displacement.y);
        if (distance <= 0.0) {
            return;
        }

        const int steps = std::max(
            1, static_cast<int>(std::ceil(distance / half_size_))
        );
        displacement.x /= steps;
        displacement.y /= steps;

        bool collided = false;
        for (int i = 0; i < steps; ++i) {
            Vec2 next = player.position;
            next.x += displacement.x;
            if (can_move(map, next)) {
                player.position.x = next.x;
            } else {
                player.velocity.x = 0.0;
                collided = true;
            }

            next = player.position;
            next.y += displacement.y;
            if (can_move(map, next)) {
                player.position.y = next.y;
            } else {
                player.velocity.y = 0.0;
                collided = true;
            }
        }

        if (collided) {
            player.dashRemaining = 0.0;
        }
    };

    const double dash_dt = std::min(dt, player.dashRemaining);
    if (dash_dt > 0.0) {
        move_with_collisions({
            player.dashDirection.x * dash_speed * dash_dt,
            player.dashDirection.y * dash_speed * dash_dt
        });
        player.dashRemaining = std::max(0.0, player.dashRemaining - dash_dt);
    }

    // If dash expires mid-frame, spend the rest of the frame moving normally.
    const double regular_dt = dt - dash_dt;
    if (regular_dt > 0.0) {
        const double response = input.drift ? 1.8 : (length > 0.0 ? 14.0 : 18.0);
        const double blend = 1.0 - std::exp(-response * regular_dt);
        player.velocity.x += (target_velocity.x - player.velocity.x) * blend;
        player.velocity.y += (target_velocity.y - player.velocity.y) * blend;

        if (length == 0.0 && std::hypot(player.velocity.x, player.velocity.y) < 0.01) {
            player.velocity = {0.0, 0.0};
        }

        move_with_collisions({
            player.velocity.x * regular_dt,
            player.velocity.y * regular_dt
        });
    }
}

}
