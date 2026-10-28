#pragma once

#include "wolf/world/Scene.hpp"

struct SDL_Window;

namespace wolf {

    class Application {
        public:
            Application() = default;
            int run();

            [[nodiscard]] const Scene& scene() const noexcept { return scene_; }

        private:
            void runLoop(SDL_Window& window);

            Scene scene_;
    };

}
