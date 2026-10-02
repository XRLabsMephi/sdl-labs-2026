#pragma once
#include "world/Player.hpp"
#include "world/Map.hpp"
#include "input/InputState.hpp"
#include <numbers>

class PlayerController {
    double move_speed_ = 1;
    double turn_speed_ = std::numbers::pi / 4;

    void apply_rotation(Player& player, const InputState& input, double delta_time) const;
    void handle_collision(Player& player, const Vector2& movement, const Map& map, double delta_time) const;
    void apply_movement(Player& player, const InputState& input, const Map& map, double delta_time) const;

public:
    PlayerController() = default;
    PlayerController(double move_speed, double turn_speed) : move_speed_(move_speed), turn_speed_(turn_speed) {}

    void update(Player& player, const InputState& input, const Map& map, double delta_time) const {
        apply_rotation(player, input, delta_time);
        apply_movement(player, input, map, delta_time);
    }
};