#include <exception>
#include <iostream>

#include <SDL3/SDL.h>

#include "wolf/core/color.hpp"
#include "wolf/render/SdlPresenter.hpp"

int main() {
    try {
        render::Framebuffer framebuffer(800, 600);
        render::SdlPresenter presenter(800, 600);
        framebuffer.clear(core::ColorRGBA{24, 28, 36});

        bool isRunning = true;
        while (isRunning) {
            SDL_Event event{};
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_EVENT_QUIT) {
                    isRunning = false;
                }
            }

            presenter.present(framebuffer);
            SDL_Delay(16);
        }

        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
