#include "wolf/app/Application.hpp"
#include <iostream>

namespace wolf::app {
    Application::Application() {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            std::cerr << "Error: " << SDL_GetError() << '\n';
            isRunning = false;
            return;
        }
        if (!SDL_CreateWindowAndRenderer("Render", windowWidth, windowHeight, 0,&window, &renderer)) {
            std::cerr << "Create window + Renderer error: " << SDL_GetError() << '\n';
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

    int Application::run() {
        if (!isRunning) {
            return 1;
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
        return 0;
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