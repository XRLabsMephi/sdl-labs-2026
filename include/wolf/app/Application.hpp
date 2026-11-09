#pragma once
#include <SDL3/SDL.h>
#include <memory>
#include "wolf/render/FrameBuffer.hpp"
#include "wolf/render/SdlPresenter.hpp"
#include "wolf/input/InputState.hpp"
#include "wolf/world/Player.hpp"
#include "wolf/world/Map.hpp"
#include "wolf/input/PlayerController.hpp"
#include "wolf/raycast/Raycaster.hpp"

namespace wolf::app {

    class Application {
    public:
        Application();
        ~Application();

        //запрет копирования приложения иначе херня
        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        int run();

    private:
        void processEvents();
        void update(double deltaTime);
        void render();

        bool isRunning = true;
        const int windowWidth = 800;
        const int windowHeight = 600;

        SDL_Window* window = nullptr;
        SDL_Renderer* renderer = nullptr;

        std::unique_ptr<render::FrameBuffer> frameBuffer;
        std::unique_ptr<render::SdlPresenter> presenter;
        input::InputState inputState;
        world::Map map;
        world::Player player;
        input::PlayerController playerController;
        raycast::Raycaster raycaster;
    };
}