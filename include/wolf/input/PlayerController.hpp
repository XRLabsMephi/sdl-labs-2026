#pragma once

#include "wolf/core/Vec2.hpp"

namespace wolf {

class Map;
struct Player;
struct InputState;

class PlayerController {
public:
    void update(Player& player, const Map& map, const InputState& input, double dt) const;
    bool canOccupy(const Map& map, Vec2 position) const;

private:
    double moveSpeed_ = 3.0;
    double turnSpeed_ = 1.8;
    double radius_ = 0.2;
};

}
