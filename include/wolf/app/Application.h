#pragma once
#include <memory>
#include <string>
#include "../input/InputState.h"
#include "../input/PlayerController.h"
#include "../world/Player.h"
#include "../world/Map.h"
namespace app {
    struct SDL_Window;
    class Renderer;

    class Application {
    private:
        SDL_Window* window = nullptr;

        std::unique_ptr<Renderer> renderer;

        world::Map map;
        world::Player player;
        input::InputState input;
        input::PlayerController playerController;

        bool running = false;

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