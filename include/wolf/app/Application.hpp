#pragma once

#include "wolf/input/PlayerController.hpp"
#include "wolf/world/Map.hpp"
#include "wolf/world/Player.hpp"

struct SDL_Window;

namespace wolf {

    class Application {
        public:
            Application();
            int run();

        private:
            void runLoop(SDL_Window& window);

            Map map_;
            Player player_;
    };

}
