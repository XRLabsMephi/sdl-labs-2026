#pragma once

#include "wolf/core/Vec2.hpp"
#include "wolf/input/InputState.hpp"
#include "wolf/world/Map.hpp"
#include "wolf/world/Player.hpp"

namespace wolf {

    class PlayerController {
    public:
        void update(
            Player& player,
            const Map& map,
            const InputState& input,
            double dt
        ) const;

    private:
        bool can_move(const Map& map, Vec2 position) const;

        double move_speed_ = 3.0;
        double rotation_speed_ = 2.0;
        double half_size_ = 0.2;
    };

}
