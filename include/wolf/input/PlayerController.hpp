#pragma once

namespace wolf
{

class Map;
struct Player;
struct InputState;

class PlayerController
{
public:
    void update(Player& player, const Map& map, const InputState& input,
                double dt) const noexcept;
};

}
