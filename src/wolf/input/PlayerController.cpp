#include "wolf/input/PlayerController.hpp"

#include <cmath>

PlayerController::PlayerController(double moveSpeed, double rotationSpeed)
    : moveSpeed(moveSpeed), rotationSpeed(rotationSpeed) {}

void PlayerController::move(Player& player, const Map& map, double x, double y) const {
    double newX = player.position.x + x;
    double newY = player.position.y + y;
    if (!map.isWall(static_cast<int>(newX), static_cast<int>(player.position.y))) {
        player.position.x = newX;
    }
    if (!map.isWall(static_cast<int>(player.position.x), static_cast<int>(newY))) {
        player.position.y = newY;
    }
}

void PlayerController::rotate(Player& player, double angle) const {
    double c = std::cos(angle);
    double s = std::sin(angle);
    double directionX = player.direction.x;
    double planeX = player.cameraPlane.x;
    player.direction.x = player.direction.x * c - player.direction.y * s;
    player.direction.y = directionX * s + player.direction.y * c;
    player.cameraPlane.x = player.cameraPlane.x * c - player.cameraPlane.y * s;
    player.cameraPlane.y = planeX * s + player.cameraPlane.y * c;
}

void PlayerController::update(Player& player, const Map& map, const InputState& input, double dt) const {
    if (!std::isfinite(dt) || dt <= 0.0) return;
    if (dt > 0.05) dt = 0.05;
    double step = moveSpeed * dt;
    if (input.forward) {
        move(player, map, player.direction.x * step, player.direction.y * step);
    }
    if (input.backward) {
        move(player, map, -player.direction.x * step, -player.direction.y * step);
    }
    if (input.left) {
        move(player, map, player.direction.y * step, -player.direction.x * step);
    }
    if (input.right) {
        move(player, map, -player.direction.y * step, player.direction.x * step);
    }
    if (input.rotateLeft) {
        rotate(player, -rotationSpeed * dt);
    }
    if (input.rotateRight) {
        rotate(player, rotationSpeed * dt);
    }
}