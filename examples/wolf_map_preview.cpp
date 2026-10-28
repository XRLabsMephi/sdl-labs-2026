#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <exception>
#include <iostream>

#include "wolf/render/FrameBuffer.hpp"
#include "wolf/render/SdlPresenter.hpp"
#include "wolf/world/Map.hpp"

int main(int, char*[]) {
    constexpr int TILE = 40;

    Map map = []() {
        try {
            return Map::load("assets/wolf/maps/map01.txt");
        } catch (const std::exception& e) {
            std::cerr << e.what() << '\n';
            throw;
        }
    }();

    const int width = map.get_width() * TILE;
    const int height = map.get_height() * TILE;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << SDL_GetError() << '\n';
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Map preview", width * 2, height * 2, 0
    );

    SDL_Renderer* renderer = window
        ? SDL_CreateRenderer(window, nullptr)
        : nullptr;

    if (!renderer) {
        std::cerr << SDL_GetError() << '\n';
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    int result = 0;

    {
        wolf::FrameBuffer frame(width, height);
        wolf::SdlPresenter presenter(renderer, width, height);

        frame.clear({20, 24, 30});

        for (int y = 0; y < map.get_height(); ++y) {
            for (int x = 0; x < map.get_width(); ++x) {
                wolf::Color color{50, 55, 65};

                switch (map.wall_id(x, y)) {
                    case 1: color = {140, 150, 160}; break;
                    case 2: color = {210, 135, 70}; break;
                    case 3: color = {165, 100, 195}; break;
                }

                frame.fillRect(
                    x * TILE + 1,
                    y * TILE + 1,
                    TILE - 2,
                    TILE - 2,
                    color
                );
            }
        }

        bool running = presenter.valid();

        if (!running) {
            std::cerr << SDL_GetError() << '\n';
            result = 1;
        }

        while (running) {
            SDL_Event event;

            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_EVENT_QUIT ||
                    (event.type == SDL_EVENT_KEY_DOWN &&
                     event.key.key == SDLK_ESCAPE)) {
                    running = false;
                }
            }

            if (running && !presenter.present(frame)) {
                std::cerr << SDL_GetError() << '\n';
                result = 1;
                break;
            }

            SDL_Delay(16);
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return result;
}
