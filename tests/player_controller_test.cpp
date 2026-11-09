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

    const double forward_distance = player.position.x - 1.5;
    check(
        forward_distance > 0.0 && forward_distance < 0.3,
        "Forward acceleration failed"
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
        std::abs(std::hypot(dx, dy) - forward_distance) < 1e-6,
        "Diagonal movement changed speed"
    );

    player = wolf::Player{};
    input = wolf::InputState{};
    input.moveForward = true;
    input.sprint = true;
    controller.update(player, map, input, 0.1);

    const double sprint_distance = player.position.x - 1.5;
    check(
        sprint_distance > forward_distance * 1.7 &&
        sprint_distance < forward_distance * 1.9,
        "Sprint speed multiplier failed"
    );

    const double before_braking = player.position.x;
    input = wolf::InputState{};
    controller.update(player, map, input, 0.1);
    check(
        player.position.x > before_braking &&
        player.position.x - before_braking < sprint_distance,
        "Movement braking failed"
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

    // Dash: instant impulse, collision-safe movement, and a cooldown.
    wolf::Map dash_map({
        {1, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1}
    });
    player = wolf::Player{};
    input = wolf::InputState{};
    input.dash = true;
    controller.update(player, dash_map, input, 0.05);
    check(
        player.position.x > 2.0 && player.dashCooldown > 0.0,
        "Dash failed to apply an impulse"
    );

    const double after_dash = player.position.x;
    player.velocity = {0.0, 0.0};
    controller.update(player, dash_map, input, 0.05);
    check(
        std::abs(player.position.x - after_dash) < 1e-9,
        "Dash ignored its cooldown"
    );

    input = wolf::InputState{};
    controller.update(player, dash_map, input, 0.9);
    input.dash = true;
    const double before_second_dash = player.position.x;
    controller.update(player, dash_map, input, 0.05);
    check(
        player.position.x > before_second_dash,
        "Dash did not recharge"
    );

    std::cout << "PlayerController tests passed\n";
}
