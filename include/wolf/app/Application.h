#pragma once
#include <memory>
#include <string>
#include "../input/InputState.h"
#include "../input/PlayerController.h"
#include "../world/Player.h"
#include "../world/Map.h"
#include <SDL3/SDL.h>
namespace app {

    class Application {
    private:
        struct WindowDeleter {
            void operator()(SDL_Window* window) const {
                SDL_DestroyWindow(window);
            }
        };

        std::unique_ptr<SDL_Window, WindowDeleter> window;

        world::Map map;
        world::Player player;
        input::InputState input;
        input::PlayerController playerController;

        bool running = false;
        bool initialized = false;
        bool sdlInitialized = false;

        void processEvents();

        void update(float deltaTime);

    public:
        Application();

        ~Application();

        Application(const Application&) = delete;

        Application& operator=(const Application&) = delete;

        bool initialize(const std::string& mapFilename);

        void run();

        void shutdown();

        bool isRunning() const {
            return running;
        }

        void stop() {
            running = false;
        }
    };
}