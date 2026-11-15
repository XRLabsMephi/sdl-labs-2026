#pragma once

#include <memory>
#include <string>

#include <SDL3/SDL.h>

#include <input/InputState.h>
#include <input/PlayerController.h>
#include <raycast/RayCaster.h>
#include <render/FrameBuffer.h>
#include <render/SdlPresenter.h>
#include <render/TextureStorage.h>
#include <world/Map.h>
#include <world/Player.h>
#include <render/SpriteRenderer.h>
#include <render/ViewSettings.h>

namespace app {

    class Application {
        struct WindowDeleter {
            void operator()(SDL_Window* window) const noexcept {
                SDL_DestroyWindow(window);
            }
        };

        std::unique_ptr<SDL_Window, WindowDeleter> window_;

        world::Map map_;
        world::Player player_;

        input::InputState input_;
        input::PlayerController playerController_;

        render::TextureStorage textureStorage_;
        std::unique_ptr<render::FrameBuffer> frameBuffer_;
        std::unique_ptr<raycast::RayCaster> rayCaster_;
        std::unique_ptr<render::SpriteRenderer> spriteRenderer_;
        std::unique_ptr<render::SdlPresenter> presenter_;
        ViewSettings viewSettings_;

        render::TextureStorage textureStorage;
        std::unique_ptr<render::FrameBuffer> frameBuffer;
        std::unique_ptr<raycast::RayCaster> rayCaster;
        std::unique_ptr<render::SpriteRenderer> spriteRenderer;
        std::unique_ptr<render::SdlPresenter> presenter;

        bool running_ = false;
        bool initialized_ = false;
        bool sdlInitialized_ = false;
        bool focused_ = false;

        void processEvents();
        void update(double deltaTime);

    public:
        Application();
        ~Application();

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        bool initialize(const std::string& mapFilename);

        void run();
        void shutdown() noexcept;

        [[nodiscard]] bool isRunning() const {
            return running_;
        }

        void stop() {
            running_ = false;
        }
    };

}