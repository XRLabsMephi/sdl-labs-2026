#include "wolf/app/Application.hpp"

#include <chrono>
#include <iostream>
#include <algorithm>
#include <filesystem>

#include <SDL3/SDL.h>

#include "wolf/core/color.hpp"

namespace {
    // Benchmark flag
    constexpr bool fBenchmark = false;

    constexpr uint32_t IdleFPSlimit = 100;
    constexpr uint32_t IdleMSwait = 1000 / IdleFPSlimit;

    // Fog consts
    constexpr double kFogStart = 3.0;
    constexpr double kFogEnd   = 14.0;

    // Player starting settings
    constexpr core::Vec2 kPlayerStartPos{2.5, 12.5};
    constexpr core::Vec2 kPlayerStartDir{1, -1};

    constexpr double RAD         = 57.2958;
    constexpr double FOV_DEGREES = 80;

    constexpr double kPlayerFov  = FOV_DEGREES / RAD;

    // PlayerController
    constexpr double kPlayerMoveSpeed = 3.0;
    constexpr double kPlayerTurnSpeed = 2.5;
    constexpr double kPlayerRadius    = 0.2; // collision radius

    // colors
    constexpr core::ColorRGBA kCeilingColor{145, 138, 101};
    constexpr core::ColorRGBA kFloorColor  {83 , 77 , 47 };
    constexpr core::ColorRGBA kFogColor    {160, 153, 107};
}

namespace app {
    Application::Application()
        : framebuffer_(kWidth, kHeight),
          presenter_(kWidth, kHeight, "Wolf Raycaster - Lab 02"),
          map_(world::Map::initMap(std::filesystem::path(WOLF_ASSETS_DIR) / "maps/map01.txt")),
          player_(kPlayerStartPos, kPlayerStartDir, kPlayerFov),
          fog_(kFogStart, kFogEnd, kFogColor),
          controller_(kPlayerMoveSpeed, kPlayerTurnSpeed, kPlayerRadius) {

        const auto root = std::filesystem::path(WOLF_ASSETS_DIR) / "textures/backrooms";
        textureStore_.load(1, root / "wallpaper.png");
        textureStore_.load(2, root / "panels.png");
        textureStore_.load(3, root / "glass.png", 96);

        const auto spritesRoot = std::filesystem::path(WOLF_ASSETS_DIR) / "sprites";
        textureStore_.load(101, spritesRoot / "barrel.png");
        textureStore_.load(102, spritesRoot / "medkit.png");
        textureStore_.load(103, spritesRoot / "lamp.png");

        sprites_ = {
            {.position = {4.5, 10.5}, .textureId = 101, .scale = 1.0},
            {.position = {5.5, 9.5},  .textureId = 102, .scale = 0.65},
            {.position = {7.3, 9.5},  .textureId = 103, .scale = 1.0},
            {.position = {11.5, 3.5}, .textureId = 101, .scale = 1.0},
        };

        raycaster_.setFog(&fog_);
    }

    Application::~Application() = default;

    int Application::run() {
        auto prevTime = std::chrono::steady_clock::now();

        int    frameCount = 0;
        double fpsTimer   = 0.0;

        // init render
        renderFrame();
        presenter_.present(framebuffer_);

        while (running_) {
            auto curTime = std::chrono::steady_clock::now();
            double dt = std::chrono::duration<double>(curTime - prevTime).count();
            prevTime = curTime;

            input::InputState input = pollInput();
            if (input.isSet(input::InputState::kQuit)) {
                running_ = false;
                break;
            }

            ++frameCount;
            fpsTimer += dt;
            if (fpsTimer >= 1.0) {
                std::cout << "FPS: " << frameCount / fpsTimer << '\n';
                frameCount = 0;
                fpsTimer   = 0.0;
            }
            
            if constexpr (fBenchmark) {
                controller_.update(input, player_, map_, dt);
                renderFrame();
                presenter_.present(framebuffer_);
            }
            else {
                if (input.empty()) {
                    if constexpr (IdleFPSlimit <= 0)
                        continue;

                    SDL_Delay(IdleMSwait);
                }
                else {
                    controller_.update(input, player_, map_, dt);
                    renderFrame();
                    presenter_.present(framebuffer_);
                }   
            }                     
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
            if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP]) {
                state.set(input::InputState::kForward);
            }
            if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN]) {
                state.set(input::InputState::kBackward);
            }
            if (keys[SDL_SCANCODE_A]) {
                state.set(input::InputState::kLeft);
            }
            if (keys[SDL_SCANCODE_D]) {
                state.set(input::InputState::kRight);
            }
            if (keys[SDL_SCANCODE_LEFT]) {
                state.set(input::InputState::kTurnLeft);
            }
            if (keys[SDL_SCANCODE_RIGHT]) {
                state.set(input::InputState::kTurnRight);
            }
        }

        return state;
    }

    void Application::renderFrame() {
        framebuffer_.clear(); // Reset per-pixel depth before the geometry passes.
        // direct access to the pixel buffer — no per-pixel bounds check
        uint32_t*      pixels = framebuffer_.color();
        const uint32_t half   = kHeight / 2;

        const uint32_t ceiling = kCeilingColor.pack();
        std::fill(pixels, pixels + (half + 1) * kWidth, ceiling); // ceil

        const uint32_t floor = kFloorColor.pack();
        std::fill(pixels + (half + 1) * kWidth, pixels + kHeight * kWidth, floor); // floor

        // walls on top layer
        raycaster_.render(player_, map_, textureStore_, framebuffer_);
        spriteRenderer_.render(player_, sprites_, textureStore_, raycaster_.depthBuffer(), framebuffer_);
        fog_.apply(framebuffer_);
    }
}
