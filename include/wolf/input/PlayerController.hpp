#pragma once
#include "wolf/world/Player.hpp"
#include "wolf/world/Map.hpp"
#include "wolf/input/InputState.hpp"

namespace wolf::input {
    class PlayerController {
     public:
        PlayerController(double speed = 3.0, double rotSpeed = 2.0);
        void update(world::Player& player, const InputState& input, const world::Map& map, double deltaTime);
     private:
        double moveSpeed;
        double rotSpeed;
    };
}