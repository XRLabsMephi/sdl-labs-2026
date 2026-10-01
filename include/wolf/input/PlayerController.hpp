#pragma once
#include "world/Player.hpp"
#include "world/Map.hpp"
#include "input/InputState.hpp"
#include <numbers>

class PlayerController {
    double move_speed_ = 1;
    double turn_speed_ = std::numbers::pi / 4;

    void apply_rotation(Player& player, const InputState& input, double delta_time) const {
        double angle = 0.0;

        if (input.turn_left) angle += turn_speed_ * delta_time;
        if (input.turn_right) angle -= turn_speed_ * delta_time;

        player.direction.rotate(angle);
        player.camera_plane.rotate(angle);
    }

    void handle_collision(Player& player, const Vector2& movement, const Map& map, double delta_time) const {
        Vector2 new_position = player.position + movement * move_speed_ * delta_time;

        if (!map.is_wall(new_position.x, player.position.y)) {
            player.position.x = new_position.x;
        }

        if (!map.is_wall(player.position.x, new_position.y)) {
            player.position.y = new_position.y;
        }
    }

    void apply_movement(Player& player, const InputState& input, const Map& map, double delta_time) const {
        Vector2 move;

        if (input.forward) move += player.direction;
        if (input.backward) move -= player.direction;

        Vector2 strafe = player.direction;
        strafe.rotate(std::numbers::pi / 2);

        if (input.left) move += strafe;
        if (input.right) move -= strafe;

        move.normalize();
        handle_collision(player, move, map, delta_time);
    }

public:
    void update(Player& player, const InputState& input, const Map& map, double delta_time) const {
        apply_rotation(player, input, delta_time);
        apply_movement(player, input, map, delta_time);
    }
};