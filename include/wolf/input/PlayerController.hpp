#pragma once

#include "wolf/input/InputState.hpp"
#include "wolf/world/Map.hpp"
#include "wolf/world/Player.hpp"

class PlayerController {
private:
    double moveSpeed;
    double rotationSpeed;

    void move(Player& player, const Map& map, double x, double y) const;
    void rotate(Player& player, double angle) const;
public:
    PlayerController(double moveSpeed = 3.0, double rotationSpeed = 2.0);
    void update(Player& player, const Map& map, const InputState& input, double dt) const;
};