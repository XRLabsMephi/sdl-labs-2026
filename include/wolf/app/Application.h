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
        SDL_Window* window = nullptr;

        SDL_Renderer* renderer = nullptr;

        world::Map map;
        world::Player player;
        input::InputState input;
        input::PlayerController playerController;

        bool running = false;
        bool initialized = false;

        void processEvents();

        void update(double deltaTime);

        void render();

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