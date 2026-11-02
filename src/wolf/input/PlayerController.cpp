#include "wolf/input/PlayerController.hpp"

#include "wolf/input/InputState.hpp"
#include "wolf/world/Map.hpp"
#include "wolf/world/Player.hpp"

#include <algorithm>
#include <cmath>

namespace wolf {

bool PlayerController::canOccupy(const Map& map, Vec2 position) const {
    if (!std::isfinite(position.x) || !std::isfinite(position.y)
        || position.x < radius_ || position.y < radius_
        || position.x > map.width() - radius_ || position.y > map.height() - radius_) {
        return false;
    }
    const int left = static_cast<int>(std::floor(position.x - radius_));
    const int right = static_cast<int>(std::floor(position.x + radius_));
    const int top = static_cast<int>(std::floor(position.y - radius_));
    const int bottom = static_cast<int>(std::floor(position.y + radius_));
    for (int y = top; y <= bottom; ++y) {
        for (int x = left; x <= right; ++x) {
            if (!map.isWall(x, y)) {
                continue;
            }
            const double dx = position.x - std::clamp(position.x, double(x), double(x + 1));
            const double dy = position.y - std::clamp(position.y, double(y), double(y + 1));
            if (dx * dx + dy * dy < radius_ * radius_) {
                return false;
            }
        }
    }
    return true;
}

void PlayerController::update(Player& player, const Map& map,
                              const InputState& input, double dt) const {
    if (!std::isfinite(dt) || dt <= 0.0) {
        return;
    }
    const double angle = (int(input.turnRight) - int(input.turnLeft)) * turnSpeed_ * dt;
    player.direction = player.direction.rotated(angle);
    player.cameraPlane = player.cameraPlane.rotated(angle);

    const Vec2 right{-player.direction.y, player.direction.x};
    Vec2 movement = player.direction * (int(input.forward) - int(input.backward))
        + right * (int(input.strafeRight) - int(input.strafeLeft));
    const double length = movement.length();
    if (length == 0.0) {
        return;
    }
    movement = movement * (moveSpeed_ * dt / length);
    const int steps = static_cast<int>(std::ceil(movement.length() / (radius_ * 0.5)));
    const Vec2 step = movement * (1.0 / steps);
    for (int i = 0; i < steps; ++i) {
        const Vec2 nextX{player.position.x + step.x, player.position.y};
        if (canOccupy(map, nextX)) {
            player.position.x = nextX.x;
        }
        const Vec2 nextY{player.position.x, player.position.y + step.y};
        if (canOccupy(map, nextY)) {
            player.position.y = nextY.y;
        }
    }
}

}
