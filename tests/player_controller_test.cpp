#include <cmath>
#include <cstdlib>
#include <iostream>

#include "wolf/input/PlayerController.hpp"

void check(bool condition, const char *message) {
    if (!condition) {
        std::cerr << message << "\n";
        std::exit(1);
    }
}

int main() {
    wolf::Map map({
        {1, 1, 1, 1, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 1, 1, 1, 1}
    });
    wolf::PlayerController controller;
    wolf::Player player;
    wolf::InputState input;
    input.moveForward = true;
    controller.update(player, map, input, 0.1);

    check(
        std::abs(player.position.x - 1.8) < 1e-6,
        "Forward movement failed"
    );
    check(
        std::abs(player.position.y - 1.5) < 1e-6,
        "Forward movement changed Y"
    );

    player = wolf::Player{};
    input.moveRight = true;
    controller.update(player, map, input, 0.1);
    double dx = player.position.x - 1.5;
    double dy = player.position.y - 1.5;

    check(
        std::abs(std::sqrt(dx * dx + dy * dy) - 0.3) < 1e-6,
        "Diagonal movement changed speed"
    );

    player = wolf::Player{};
    input = wolf::InputState{};
    input.turnRight = true;
    controller.update(player, map, input, 0.5);

    check(
        std::abs(player.direction.x - std::cos(1.0)) < 1e-6 &&
        std::abs(player.direction.y - std::sin(1.0)) < 1e-6,
        "Camera rotation failed"
    );

    double dot =
            player.direction.x * player.cameraPlane.x +
            player.direction.y * player.cameraPlane.y;

    check(
        std::abs(dot) < 1e-6,
        "Camera vectors are not perpendicular"
    );

    player = wolf::Player{};
    input = wolf::InputState{};
    input.moveForward = true;
    controller.update(player, map, input, 1.0);

    check(
        player.position.x > 1.5 &&
        player.position.x + 0.2 < 4.0,
        "Player crossed a wall"
    );

    // Valid spawn.
    check(
        controller.can_move(map, {1.5, 1.5}),
        "Valid spawn rejected"
    );

    // Spawn inside a wall.
    check(
        !controller.can_move(map, {0.5, 0.5}),
        "Spawn inside wall accepted"
    );

    // Spawn too close to a wall.
    check(
        !controller.can_move(map, {1.1, 1.5}),
        "Spawn overlapping wall accepted"
    );

    // Spawn outside the map.
    check(
        !controller.can_move(map, {-1.0, 1.5}),
        "Spawn outside map accepted"
    );

    std::cout << "PlayerController tests passed\n";
}
