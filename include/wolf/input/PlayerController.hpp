#pragma once

#include "wolf/input/InputState.hpp"
#include "wolf/world/Map.hpp"
#include "wolf/world/Player.hpp"

namespace input {
    class PlayerController {
    public:
        PlayerController(double moveSpeed, double turnSpeed, double radius);

        void update(const InputState &input, world::Player &player,
                    const world::Map &map, double dt);

    private:
        [[nodiscard]] bool canStand(core::Vec2 pos, const world::Map &map) const;

        double moveSpeed_;
        double turnSpeed_;
        double radius_;
    };
}
