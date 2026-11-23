#include "wolf/app/Application.hpp"

#include "wolf/input/InputState.hpp"
#include "wolf/input/PlayerController.hpp"
#include "wolf/raycast/Raycaster.hpp"
#include "wolf/render/Fog.hpp"
#include "wolf/render/FrameBuffer.hpp"
#include "wolf/render/SdlPresenter.hpp"
#include "wolf/render/SpriteRenderer.hpp"
#include "wolf/render/TextureStore.hpp"
#include "wolf/world/Map.hpp"
#include "wolf/world/Player.hpp"

#include <SDL3/SDL.h>

#include <chrono>
#include <stdexcept>
#include <vector>

namespace wolf
{
namespace
{

void resetPlayerIfInsideWall(Player& player, const Map& map)
{
    const int startX = static_cast<int>(player.position.x);
    const int startY = static_cast<int>(player.position.y);
    if (!map.isWall(startX, startY))
    {
        return;
    }

    for (int y = 0; y < map.height(); ++y)
    {
        for (int x = 0; x < map.width(); ++x)
        {
            if (!map.isWall(x, y))
            {
                player.position = {x + 0.5, y + 0.5};
                player.direction = {-1.0, 0.0};
                player.cameraPlane = {0.0, 0.66};
                return;
            }
        }
    }

    throw std::runtime_error("Map does not contain a free spawn cell");
}

InputState readInput()
{
    InputState input;
    int keyCount = 0;
    const bool* keys = SDL_GetKeyboardState(&keyCount);
    const auto pressed = [keys, keyCount](SDL_Scancode key)
    {
        return static_cast<int>(key) < keyCount && keys[key];
    };

    input.moveForward = pressed(SDL_SCANCODE_W);
    input.moveBackward = pressed(SDL_SCANCODE_S);
    input.strafeLeft = pressed(SDL_SCANCODE_A);
    input.strafeRight = pressed(SDL_SCANCODE_D);
    input.turnLeft = pressed(SDL_SCANCODE_LEFT) || pressed(SDL_SCANCODE_Q);
    input.turnRight = pressed(SDL_SCANCODE_RIGHT) || pressed(SDL_SCANCODE_E);
    return input;
}

} // namespace

int Application::run(bool textured)
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        throw std::runtime_error(SDL_GetError());
    }

    SDL_Window* window = SDL_CreateWindow(textured ? "Software Raycaster - Lab 2" : "Software Raycaster - Lab 1",
                                          960, 600, SDL_WINDOW_RESIZABLE);
    if (window == nullptr)
    {
        const char* error = SDL_GetError();
        SDL_Quit();
        throw std::runtime_error(error);
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (renderer == nullptr)
    {
        const char* error = SDL_GetError();
        SDL_DestroyWindow(window);
        SDL_Quit();
        throw std::runtime_error(error);
    }

    try
    {
        const Map map = Map::loadFromFile(textured ? "assets/wolf/maps/map02.txt" : "assets/wolf/maps/map01.txt");
        Player player;
        TextureStore textures;
        std::vector<Sprite> sprites;
        if (textured)
        {
            player.position = {8.5, 5.5};
            textures.loadBmp(1, "assets/wolf/textures/brick.bmp");
            textures.loadBmp(2, "assets/wolf/textures/metal.bmp");
            textures.loadBmp(3, "assets/wolf/textures/concrete.bmp");
            textures.loadBmp(4, "assets/wolf/textures/barrel.bmp", true);
            textures.loadBmp(5, "assets/wolf/textures/lamp.bmp", true);
            sprites = {{{6.8, 4.85}, 4, 0.65}, {{4.3, 6.22}, 5, 1.0},
                       {{2.0, 3.1}, 4, 0.85}, {{1.6, 7.7}, 5, 0.9}};
        }
        resetPlayerIfInsideWall(player, map);
        FrameBuffer frame(320, 200);
        PlayerController controller;
        Raycaster raycaster;
        SpriteRenderer spriteRenderer;
        const Fog fog{2.0, 8.0};
        std::vector<double> wallDepths(frame.width());
        SdlPresenter presenter(renderer);
        bool running = true;
        auto previous = std::chrono::steady_clock::now();

        while (running)
        {
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_EVENT_QUIT)
                {
                    running = false;
                }
            }

            const auto now = std::chrono::steady_clock::now();
            const double deltaSeconds = std::chrono::duration<double>(now - previous).count();
            previous = now;

            const InputState input = readInput();
            controller.update(player, map, input, deltaSeconds);
            if (textured)
            {
                raycaster.renderTextured(map, player, textures, fog, frame, wallDepths);
                spriteRenderer.render(sprites, player, textures, wallDepths, fog, frame);
            }
            else
            {
                raycaster.render(map, player, frame);
            }
            presenter.present(frame);
        }
    }
    catch (...)
    {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        throw;
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

} // namespace wolf
