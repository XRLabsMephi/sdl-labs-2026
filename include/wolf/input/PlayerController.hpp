#pragma once

#include "wolf/input/InputState.hpp"
#include "wolf/world/Map.hpp"
#include "wolf/world/Player.hpp"

namespace wolf
{
class PlayerController
{
public:
    void update(Player& player, const Map& map, const InputState& input, double deltaTime) const;

private:
    bool canStand(const Map& map, double x, double y) const;
    void rotate(Player& player, double angle) const;
};
}
