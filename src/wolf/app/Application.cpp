#include "wolf/app/Application.hpp"

#include <format>
#include <iostream>

namespace wolf::app {
    Application::Application() {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            std::cerr << "Error: " << SDL_GetError() << '\n';
            isRunning = false;
            return;
        }
        //SDL_WINDOW_RESIZABLE | SDL_WINDOW_UTILITY | SDL_WINDOW_BORDERLESS
        if (!SDL_CreateWindowAndRenderer("Render", windowWidth, windowHeight, SDL_WINDOW_RESIZABLE | SDL_WINDOW_UTILITY, &window, &renderer)) {
            throw std::runtime_error(std::format("Could not create window: {}", SDL_GetError()));
            isRunning = false;
            return;
        }
        frameBuffer = std::make_unique<render::FrameBuffer>(windowWidth, windowHeight);
        presenter = std::make_unique<render::SdlPresenter>(renderer, windowWidth, windowHeight);
    }

    Application::~Application() {
        presenter.reset();
        frameBuffer.reset();
        if (renderer) {
            SDL_DestroyRenderer(renderer);
        }
        if (window) {
            SDL_DestroyWindow(window);
        }
        SDL_Quit();
    }

    void Application::run() {
        if (!isRunning) {
            throw std::runtime_error("Application is not running");
        }
        std::uint64_t lastTime = SDL_GetTicks();
        //должно идти с одной скорость в сек иначе фигня
        while (isRunning) {
            const std::uint64_t cur = SDL_GetTicks();
            const double delta = (cur - lastTime) / 1000.0;
            lastTime = cur;
            processEvents();
            update(delta);
            render();
        }
    }

    void Application::processEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                isRunning = false;
            } else if (event.type == SDL_EVENT_KEY_DOWN) {
                // в SDL3 event.key.key
                if (event.key.key == SDLK_ESCAPE) {
                    isRunning = false;
                }
            }
        }
        int keys;
        const bool* keyboardState = SDL_GetKeyboardState(&keys);

        inputState.moveForward = keyboardState[SDL_SCANCODE_W];
        inputState.moveBackward = keyboardState[SDL_SCANCODE_S];
        inputState.moveLeft = keyboardState[SDL_SCANCODE_A];
        inputState.moveRight = keyboardState[SDL_SCANCODE_D];

        inputState.turnLeft = keyboardState[SDL_SCANCODE_LEFT];
        inputState.turnRight = keyboardState[SDL_SCANCODE_RIGHT];
    }

    void Application::update(double deltaTime) {
        playerController.update(player, inputState, map, deltaTime);
    }

    void Application::render() {
        raycaster.render(*frameBuffer, player, map);
        presenter->present(*frameBuffer);
    }

}