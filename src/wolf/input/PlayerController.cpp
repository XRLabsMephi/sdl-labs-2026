#include <stdexcept>
#include "input/PlayerController.hpp"

void PlayerController::apply_rotation(Player &player, const InputState &input, double delta_time) const {
    double angle = 0.0;

    if (input.turn_left) angle -= turn_speed_ * delta_time;
    if (input.turn_right) angle += turn_speed_ * delta_time;

    player.direction.rotate(angle);
    player.camera_plane.rotate(angle);
}

void PlayerController::handle_collision(Player &player, const Vector2 &movement, const Map &map, double delta_time) const {
    Vector2 new_position = player.position + movement * move_speed_ * delta_time;

    if (!map.is_wall(new_position.x, player.position.y)) {
        player.position.x = new_position.x;
    }

    if (!map.is_wall(player.position.x, new_position.y)) {
        player.position.y = new_position.y;
    }
}

void PlayerController::apply_movement(Player &player, const InputState &input, const Map &map, double delta_time) const {
    Vector2 move;

    if (input.forward) move += player.direction;
    if (input.backward) move -= player.direction;

    Vector2 strafe = player.direction;
    strafe.rotate(std::numbers::pi / 2);

    if (input.left) move -= strafe;
    if (input.right) move += strafe;

    move.normalize();
    handle_collision(player, move, map, delta_time);
}

void PlayerController::resolve_spawn(Player &player, const Map &map) {
    double player_x = player.position.x;
    double player_y = player.position.y;

    if (!map.is_wall(player_x, player_y)) return;

    auto new_position = map.find_nearest_free_cell(player_x, player_y);

    if (!new_position) {
        throw std::runtime_error("Map has no free cells");
    }

    player.position.x = new_position->first + 0.5;
    player.position.y = new_position->second + 0.5;
}