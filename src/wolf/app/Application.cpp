#include "wolf/app/Application.hpp"

#include <chrono>
#include <iostream>

#include <SDL3/SDL.h>

#include "wolf/core/color.hpp"

namespace app {
    Application::Application()
        : framebuffer_(kWidth, kHeight),
          presenter_(kWidth, kHeight, "Wolf Raycaster - Lab 01"),
          map_(world::Map::defaultMap()),
          player_(core::Vec2{2.5, 2.5}, core::Vec2{1.0, 0.0}, 1.15),
          controller_(3.0, 2.5, 0.2) {}

    Application::~Application() = default;

    int Application::run() {
        auto prevTime = std::chrono::steady_clock::now();

        while (running_) {
            auto curTime = std::chrono::steady_clock::now();
            double dt = std::chrono::duration<double>(curTime - prevTime).count();
            prevTime = curTime;

            input::InputState input = pollInput();
            if (input.isSet(input::InputState::kQuit)) {
                running_ = false;
                break;
            }

            controller_.update(input, player_, map_, dt);
            renderFrame();
            presenter_.present(framebuffer_);
        }

        return 0;
    }

    input::InputState Application::pollInput() {
        input::InputState state{};

        SDL_Event event{};
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                state.set(input::InputState::kQuit);
            }
            if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
                state.set(input::InputState::kQuit);
            }
        }

        const bool *keys = SDL_GetKeyboardState(nullptr);
        if (keys) {
            if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP])
                state.set(input::InputState::kForward);
            if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN])
                state.set(input::InputState::kBackward);
            if (keys[SDL_SCANCODE_A])
                state.set(input::InputState::kLeft);
            if (keys[SDL_SCANCODE_D])
                state.set(input::InputState::kRight);
            if (keys[SDL_SCANCODE_LEFT])
                state.set(input::InputState::kTurnLeft);
            if (keys[SDL_SCANCODE_RIGHT])
                state.set(input::InputState::kTurnRight);
        }

        return state;
    }

    void Application::renderFrame() {
        //Потолок и пол
        core::ColorRGBA ceiling{60, 60, 80};
        core::ColorRGBA floor{40, 40, 40};

        int half = kHeight / 2;

        for (int y = 0; y < kHeight; ++y) {
            core::ColorRGBA rowColor = (y < half) ? ceiling : floor;
            for (int x = 0; x < kWidth; ++x) {
                framebuffer_.setPixel(x, y, rowColor);
            }
        }
    }
}
