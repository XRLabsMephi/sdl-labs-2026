#pragma once

namespace wolf
{

class Map;
struct Player;
struct InputState;

class PlayerController
{
public:
    // dt is in seconds. Invalid/nonpositive dt is ignored; pauses over one
    // second are limited to one second. Speeds: 3 units/s and 1.8 radians/s.
    // The player occupies an axis-aligned square with half-size 0.2 units.
    void update(Player& player, const Map& map, const InputState& input,
                double dt) const noexcept;
};

}
