#include <world/Player.h>

#include <cmath>
#include <numbers>
#include <stdexcept>

namespace {

    constexpr double FULL_TURN = 2.0 * std::numbers::pi;

}

void world::Player::rotate(double angleDelta) {
    if (!std::isfinite(angleDelta)) {
        throw std::invalid_argument("Invalid player rotation");
    }

    angleDelta = std::fmod(angleDelta, FULL_TURN);
    rotation = std::fmod(rotation + angleDelta, FULL_TURN);

    if (rotation < 0.0) {
        rotation += FULL_TURN;
    }
}

core::Vector2 world::Player::getDirection() const {
    return {
        std::cos(rotation),
        std::sin(rotation)
    };
}