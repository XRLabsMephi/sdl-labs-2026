#include "wolf/app/Application.hpp"

#include "wolf/input/InputState.hpp"

#include <SDL3/SDL.h>

#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

namespace wolf {

    namespace {

        struct SdlCleanup {
            ~SdlCleanup() {
                SDL_Quit();
            }
        };

        std::runtime_error sdlError(const char* operation) {
            return std::runtime_error(std::string(operation) + ": " + SDL_GetError());
        }

        bool isQuitEvent(const SDL_Event& event, SDL_WindowID windowId) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    return true;
                case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                    return event.window.windowID == windowId;
                case SDL_EVENT_KEY_DOWN:
                    return event.key.windowID == windowId &&
                           event.key.scancode == SDL_SCANCODE_ESCAPE;
                default:
                    return false;
            }
        }

        InputState readInput(SDL_Window& window) {
            InputState input;
            const SDL_WindowID windowId = SDL_GetWindowID(&window);
            SDL_Event event{};
            while (SDL_PollEvent(&event)) {
                input.quit |= isQuitEvent(event, windowId);
            }
            const SDL_WindowFlags flags = SDL_GetWindowFlags(&window);
            if ((flags & SDL_WINDOW_INPUT_FOCUS) && !(flags & SDL_WINDOW_MINIMIZED)) {
                const bool* keys = SDL_GetKeyboardState(nullptr);
                input.moveForward = keys[SDL_SCANCODE_W];
                input.moveBackward = keys[SDL_SCANCODE_S];
                input.strafeLeft = keys[SDL_SCANCODE_A];
                input.strafeRight = keys[SDL_SCANCODE_D];
                input.turnLeft = keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_Q];
                input.turnRight = keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_E];
                input.quit = input.quit || keys[SDL_SCANCODE_ESCAPE];
            }
            return input;
        }

        std::string diagnosticsTitle(const Player& player) {
            std::ostringstream title;
            title << std::fixed << std::setprecision(2)
                  << "Wolf Lab 1 | renderer not connected"
                  << " | pos " << player.position.x << ", " << player.position.y
                  << " | dir " << player.direction.x << ", " << player.direction.y
                  << " | WASD, arrows/Q/E, Esc";
            return title.str();
        }

        void showDiagnostics(SDL_Window& window, const Player& player) {
            if (!SDL_SetWindowTitle(&window, diagnosticsTitle(player).c_str())) {
                throw sdlError("SDL_SetWindowTitle");
            }
            SDL_Surface* surface = SDL_GetWindowSurface(&window);
            if (!surface) {
                throw sdlError("SDL_GetWindowSurface");
            }
            if (!SDL_FillSurfaceRect(surface, nullptr, SDL_MapSurfaceRGB(surface, 24, 24, 28))) {
                throw sdlError("SDL_FillSurfaceRect");
            }
            if (!SDL_UpdateWindowSurface(&window)) {
                throw sdlError("SDL_UpdateWindowSurface");
            }
        }

    }

    Application::Application() : map_(makeDefaultMap()) {}

    int Application::run() {
        try {
            const SdlCleanup cleanup;
            if (!SDL_Init(SDL_INIT_VIDEO)) {
                throw sdlError("SDL_Init");
            }
            const std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window(
                SDL_CreateWindow("Wolf Lab 1", 960, 600, SDL_WINDOW_RESIZABLE),
                SDL_DestroyWindow
            );
            if (!window) {
                throw sdlError("SDL_CreateWindow");
            }
            runLoop(*window);
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "Application: " << error.what() << '\n';
            return 1;
        }
    }

    void Application::runLoop(SDL_Window& window) {
        Uint64 previousTime = SDL_GetTicksNS();
        constexpr Uint64 targetFrameTime = 1'000'000'000 / 60;
        while (true) {
            const Uint64 frameStart = SDL_GetTicksNS();
            const double dt = static_cast<double>(frameStart - previousTime) / 1e9;
            previousTime = frameStart;
            const InputState input = readInput(window);
            if (input.quit) {return;}
            PlayerController::update(player_, map_, input, dt);
            if (!(SDL_GetWindowFlags(&window) & SDL_WINDOW_MINIMIZED)) {
                showDiagnostics(window, player_);
            }
            const Uint64 elapsed = SDL_GetTicksNS() - frameStart;
            if (elapsed < targetFrameTime) {
                SDL_DelayNS(targetFrameTime - elapsed);
            }
        }
    }

}
