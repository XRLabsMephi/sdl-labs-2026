#pragma once

namespace wolf {

    class Map;
    struct Player;
    struct InputState;

    class PlayerController {
        public:
            static void update(Player& player, const Map& map, const InputState& input, double dt) noexcept;
    };

}
