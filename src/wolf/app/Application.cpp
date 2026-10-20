#include "wolf/app/Application.hpp"

#include <chrono>
#include <iostream>

#include <SDL3/SDL.h>

#include "wolf/core/color.hpp"

namespace {
    //Player starting settings
    constexpr core::Vec2 kPlayerStartPos{2.5, 2.5};
    constexpr core::Vec2 kPlayerStartDir{1.0, 0.0};
    
    constexpr double RAD         = 57.2958;
    constexpr double FOV_DEGREES = 80;

    constexpr double kPlayerFov  = FOV_DEGREES / RAD;

    //PlayerController
    constexpr double kPlayerMoveSpeed = 3.0;
    constexpr double kPlayerTurnSpeed = 2.5;
    constexpr double kPlayerRadius    = 0.2; // collision radius

    //colors
    constexpr core::ColorRGBA kCeilingColor{60, 60, 80};
    constexpr core::ColorRGBA kFloorColor  {40, 40, 40};
}

namespace app {
    Application::Application()
        : framebuffer_(kWidth, kHeight),
          presenter_(kWidth, kHeight, "Wolf Raycaster - Lab 01"),
          map_(world::Map::defaultMap()),
          player_(kPlayerStartPos, kPlayerStartDir, kPlayerFov),
          controller_(kPlayerMoveSpeed, kPlayerTurnSpeed, kPlayerRadius) {}

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
            if (event.type == SDL_EVENT_QUIT)
                state.set(input::InputState::kQuit);
            
            if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) 
                state.set(input::InputState::kQuit);
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
        framebuffer_.clear(kCeilingColor);

        const int half = kHeight / 2;
        for (int y = half; y < kHeight; ++y) {
            for (int x = 0; x < kWidth; ++x) {
                framebuffer_.setPixel(x, y, kFloorColor);
            }
        }

        raycaster_.render(player_, map_, framebuffer_);
    }
}
