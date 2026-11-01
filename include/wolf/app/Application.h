#pragma once

#include <memory>
#include <string>

#include <SDL3/SDL.h>

#include <input/InputState.h>
#include <input/PlayerController.h>
#include <raycast/RayCaster.h>
#include <render/FrameBuffer.h>
#include <render/SdlPresenter.h>
#include <world/Map.h>
#include <world/Player.h>

namespace app {

    class Application {
    private:
        struct WindowDeleter {
            void operator()(SDL_Window* window) const noexcept {
                SDL_DestroyWindow(window);
            }
        };

        std::unique_ptr<SDL_Window, WindowDeleter> window;

        world::Map map;
        world::Player player;

        input::InputState input;
        input::PlayerController playerController;

        std::unique_ptr<render::FrameBuffer> frameBuffer;
        std::unique_ptr<raycast::RayCaster> rayCaster;
        std::unique_ptr<render::SdlPresenter> presenter;

        bool running = false;
        bool initialized = false;
        bool sdlInitialized = false;
        bool focused = false;

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

        bool isRunning() const {
            return running;
        }

        void stop() {
            running = false;
        }
    };

}