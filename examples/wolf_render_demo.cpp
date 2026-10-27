#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <cmath>
#include <iostream>

#include "wolf/render/FrameBuffer.hpp"
#include "wolf/render/SdlPresenter.hpp"

int main(int argc, char* argv[]) {
    constexpr int W = 320;
    constexpr int H = 200;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << SDL_GetError() << '\n';
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "FrameBuffer demo", W * 2, H * 2, 0
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
        wolf::FrameBuffer frame(W, H);
        wolf::SdlPresenter presenter(renderer, W, H);

        frame.clear({90, 130, 180});
        frame.fillRect(0, H / 2, W, H / 2, {65, 65, 65});

        for (int x = 0; x < W; ++x) {
            int h = 30 + (W / 2 - std::abs(x - W / 2)) / 3;

            wolf::Color color = (x / 40) % 2
                ? wolf::Color{160, 75, 60}
                : wolf::Color{195, 100, 75};

            frame.drawColumn(x, H / 2 - h, H / 2 + h, color);
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
                    examples/wolf_render_demo.cpp(event.type == SDL_EVENT_KEY_DOWN &&
                     event.key.key == SDLK_ESCAPE))
                    running = false;
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
