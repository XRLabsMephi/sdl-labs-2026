#pragma once

#include "wolf/input/InputState.hpp"

namespace wolf
{

class Map;
struct Player;

class PlayerController
{
public:
    explicit PlayerController(double moveSpeed = 3.0, double rotationSpeed = 2.2);

    void update(Player& player, const Map& map, const InputState& input, double deltaSeconds) const;

private:
    void move(Player& player, const Map& map, double offsetX, double offsetY) const;

    double moveSpeed_;
    double rotationSpeed_;
    double collisionRadius_ = 0.18;
};

} // namespace wolf
