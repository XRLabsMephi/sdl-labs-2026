#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

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

    // A dash covers the same distance at different frame rates.
    std::vector<std::vector<int>> dash_cells(3, std::vector<int>(30, 0));
    for (int x = 0; x < 30; ++x) {
        dash_cells[0][x] = 1;
        dash_cells[2][x] = 1;
    }
    for (auto& row : dash_cells) {
        row.front() = 1;
        row.back() = 1;
    }
    const wolf::Map dash_map(dash_cells);

    player = wolf::Player{};
    input = wolf::InputState{};
    input.dash = true;
    controller.update(player, dash_map, input, 0.05);
    check(
        player.position.x > 2.0 &&
        std::abs(player.dashRemaining - 0.10) < 1e-9,
        "Dash did not start at the intended speed and duration"
    );

    input.dash = false;
    controller.update(player, dash_map, input, 0.05);
    controller.update(player, dash_map, input, 0.05);
    const double stepped_position = player.position.x;
    check(
        std::abs(stepped_position - 3.6) < 1e-6 &&
        player.dashRemaining < 1e-9,
        "Dash did not travel a fixed 150 ms distance"
    );

    wolf::Player single_frame_player;
    input.dash = true;
    controller.update(single_frame_player, dash_map, input, 0.15);
    check(
        std::abs(single_frame_player.position.x - stepped_position) < 1e-6,
        "Dash distance depends on frame rate"
    );

    // Repeated dash requests during cooldown must not restart the dash.
    input.dash = true;
    controller.update(player, dash_map, input, 0.05);
    check(
        player.dashRemaining == 0.0 && player.dashCooldown > 0.0,
        "Dash ignored its cooldown"
    );

    input.dash = false;
    controller.update(player, dash_map, input, 0.9);
    input.dash = true;
    controller.update(player, dash_map, input, 0.05);
    check(
        player.dashRemaining > 0.0,
        "Dash did not recharge"
    );

    // A dash into a wall must stop at the collision boundary.
    wolf::Map dash_wall_map({
        {1, 1, 1, 1, 1},
        {1, 0, 0, 1, 1},
        {1, 1, 1, 1, 1}
    });
    player = wolf::Player{};
    input = wolf::InputState{};
    input.dash = true;
    controller.update(player, dash_wall_map, input, 0.15);
    check(
        player.position.x <= 2.8 + 1e-9 &&
        player.dashRemaining == 0.0,
        "Dash passed through a wall"
    );

    // Sustained sprint grows from 1.8x to 2.5x on a long straight.
    std::vector<std::vector<int>> runway(3, std::vector<int>(100, 0));
    for (int x = 0; x < 100; ++x) {
        runway[0][x] = 1;
        runway[2][x] = 1;
    }
    for (auto& row : runway) {
        row.front() = 1;
        row.back() = 1;
    }
    const wolf::Map long_map(runway);
    player = wolf::Player{};
    input = wolf::InputState{};
    input.moveForward = true;
    input.sprint = true;
    for (int i = 0; i < 50; ++i) {
        controller.update(player, long_map, input, 0.1);
    }
    check(
        player.sprintTime > 4.9 &&
        std::hypot(player.velocity.x, player.velocity.y) > 7.4,
        "Sustained sprint never reached its maximum speed"
    );

    input.sprint = false;
    controller.update(player, long_map, input, 0.1);
    check(
        player.sprintTime == 0.0 &&
        std::hypot(player.velocity.x, player.velocity.y) < 7.5,
        "Sustained sprint did not reset"
    );

    std::cout << "PlayerController tests passed\n";
}
