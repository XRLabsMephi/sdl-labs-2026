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

    double length = std::sqrt(
        move.x * move.x + move.y * move.y
    );

    if (length == 0.0) {
        return;
    }

    // 3. Нормализация: движение по диагонали
    // не должно быть быстрее обычного.
    move.x = move.x / length * move_speed_ * dt;
    move.y = move.y / length * move_speed_ * dt;

    // 4. Делим перемещение на небольшие части,
    // чтобы при большом dt не перескочить стену.
    double distance = move_speed_ * dt;
    int steps = std::max(
        1,
        static_cast<int>(std::ceil(distance / half_size_))
    );
    move.x /= steps;
    move.y /= steps;

    // 5. Проверяем X и Y независимо.
    for (int i = 0; i < steps; ++i) {
        Vec2 next = player.position;
        next.x += move.x;
        if (can_move(map, next)) {
            player.position.x = next.x;
        }
        next = player.position;
        next.y += move.y;
        if (can_move(map, next)) {
            player.position.y = next.y;
        }
    }
}

}
