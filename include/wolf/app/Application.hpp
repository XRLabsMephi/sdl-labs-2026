#pragma once

#include "wolf/input/InputState.hpp"
#include "wolf/input/PlayerController.hpp"
#include "wolf/render/SdlPresenter.hpp"
#include "wolf/render/framebuffer.hpp"
#include "wolf/world/Map.hpp"
#include "wolf/world/Player.hpp"

namespace app {
    // Владеет всем жизненным циклом
    // Запуск через run()
    class Application {
    public:
        Application();
        ~Application();

        Application(const Application &) = delete;
        Application &operator=(const Application &) = delete;

        int run();

    private:
        [[nodiscard]] input::InputState pollInput();

        void renderFrame();

        static constexpr int kWidth = 800;
        static constexpr int kHeight = 600;

        render::Framebuffer framebuffer_;
        render::SdlPresenter presenter_;
        world::Map map_;
        world::Player player_;
        input::PlayerController controller_;
        bool running_ = true;
    };
}
