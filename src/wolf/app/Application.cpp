#include "wolf/app/Application.hpp"

#include <SDL3/SDL.h>

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace wolf
{
bool Application::initialize()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "SDL error: " << SDL_GetError() << '\n';
        return false;
    }

    if (!SDL_CreateWindowAndRenderer("Raycaster - Lab 1", SCREEN_W, SCREEN_H, 0, &window_, &renderer_))
    {
        std::cerr << "SDL error: " << SDL_GetError() << '\n';
        SDL_Quit();
        return false;
    }

    if (!map_.loadFromFile("assets/wolf/maps/map01.txt"))
    {
        std::cerr << "Map load error: assets/wolf/maps/map01.txt\n";
        shutdown();
        return false;
    }

    if (!presenter_.initialize(renderer_, SCREEN_W, SCREEN_H))
    {
        std::cerr << "SDL texture error: " << SDL_GetError() << '\n';
        shutdown();
        return false;
    }

    return true;
}

void Application::shutdown()
{
    presenter_.shutdown();

    if (renderer_)
    {
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }

    if (window_)
    {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }

    SDL_Quit();
}

InputState Application::readInput(bool& running)
{
    InputState input;

    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT)
        {
            running = false;
            input.quit = true;
        }

        if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE)
        {
            running = false;
            input.quit = true;
        }
    }

    const bool* keys = SDL_GetKeyboardState(nullptr);

    input.forward = keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP];
    input.backward = keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN];
    input.strafeLeft = keys[SDL_SCANCODE_Q];
    input.strafeRight = keys[SDL_SCANCODE_E];
    input.turnLeft = keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT];
    input.turnRight = keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT];

    return input;
}

void Application::updateTitle()
{
    const double angle = std::atan2(player_.direction.y, player_.direction.x);

    std::ostringstream title;

    title << std::fixed
          << std::setprecision(2)
          << "Raycaster - Lab 1"
          << " | Pos=(" << player_.position.x << "," << player_.position.y << ")"
          << " | Angle=" << angle
          << " | W/S move"
          << " | A/D rotate"
          << " | Q/E strafe";

    SDL_SetWindowTitle(window_, title.str().c_str());
}

int Application::run()
{
    if (!initialize())
    {
        return 1;
    }

    bool running = true;
    Uint64 previousTicks = SDL_GetTicks();

    while (running)
    {
        const Uint64 currentTicks = SDL_GetTicks();
        double deltaTime = static_cast<double>(currentTicks - previousTicks) / 1000.0;
        previousTicks = currentTicks;

        deltaTime = std::min(deltaTime, 0.05);

        const InputState input = readInput(running);

        if (!running || input.quit)
        {
            break;
        }

        controller_.update(player_, map_, input, deltaTime);
        raycaster_.render(map_, player_, frameBuffer_);

        if (!presenter_.present(renderer_, frameBuffer_))
        {
            std::cerr << "SDL present error: " << SDL_GetError() << '\n';
            running = false;
        }

        updateTitle();
        SDL_Delay(1);
    }

    shutdown();
    return 0;
}
}
