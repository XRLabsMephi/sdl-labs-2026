#pragma once

#include <vector>

#include "wolf/effects/Fog.hpp"
#include "wolf/input/InputState.hpp"
#include "wolf/input/PlayerController.hpp"
#include "wolf/raycast/Raycaster.hpp"
#include "wolf/render/SdlPresenter.hpp"
#include "wolf/render/framebuffer.hpp"
#include "wolf/sprite/SpriteRenderer.hpp"
#include "wolf/texture/TextureStore.hpp"
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

        static constexpr int kWidth = 960;
        static constexpr int kHeight = 720;

        render::Framebuffer framebuffer_;
        render::SdlPresenter presenter_;
        world::Map map_;
        world::Player player_;
        raycast::Raycaster raycaster_;
        texture::TextureStore textureStore_;
        std::vector<sprite::Sprite> sprites_; // Populated by participant 1's scene setup.
        sprite::SpriteRenderer spriteRenderer_;
        effects::Fog fog_;
        input::PlayerController controller_;
        bool running_ = true;
    };
}
