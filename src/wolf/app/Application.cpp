#include "wolf/app/Application.hpp"

#include <SDL3/SDL.h>
#include <exception>
#include <iostream>

#include "wolf/input/PlayerController.hpp"
#include "wolf/input/SdlInput.hpp"
#include "wolf/raycast/Raycaster.hpp"
#include "wolf/render/FrameBuffer.hpp"
#include "wolf/render/SdlPresenter.hpp"
#include "wolf/world/Map.hpp"
#include "wolf/world/Player.hpp"

namespace wolf {

Application::~Application() {
    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
    }

    if (window_) {
        SDL_DestroyWindow(window_);
    }

    if (sdl_initialized_) {
        SDL_Quit();
    }
}

bool Application::init() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init: " << SDL_GetError() << "\n";
        return false;
    }

    sdl_initialized_ = true;
    window_ = SDL_CreateWindow(
        "Blobick Raycaster",
        width_,
        height_,
        0
    );

    if (!window_) {
        std::cerr << "SDL_CreateWindow: " << SDL_GetError() << "\n";
        return false;
    }

    renderer_ = SDL_CreateRenderer(window_, nullptr);
    if (!renderer_) {
        std::cerr << "SDL_CreateRenderer: " << SDL_GetError() << "\n";
        return false;
    }

    return true;
}

int Application::run() {
    if (!init()) {
        return 1;
    }

    try {
        return run_loop();
    } catch (const std::exception& error) {
        std::cerr << error.what() << "\n";
        return 1;
    }
}

int Application::run_loop() {
    Map map = Map::load("assets/wolf/maps/map01.txt");
    Player player;
    PlayerController controller;
    FrameBuffer frame(width_, height_);
    Raycaster raycaster;
    SdlPresenter presenter(renderer_, width_, height_);

    if (!presenter.valid()) {
        std::cerr << "SdlPresenter: "
                  << SDL_GetError() << "\n";
        return 1;
    }

    Uint64 previous_time = SDL_GetTicksNS();
    while (true) {
        InputState input = read_input();
        if (input.quit) {
            break;
        }
        Uint64 current_time = SDL_GetTicksNS();
        double dt = static_cast<double>(current_time - previous_time) / 1'000'000'000.0;
        previous_time = current_time;

        // После долгой паузы не перемещаем игрока
        // сразу на большое расстояние.
        if (dt > 0.1) {
            dt = 0.1;
        }

        controller.update(player, map, input, dt);

        // Потолок.
        frame.clear({90, 130, 180});

        // Пол.
        frame.fillRect(
            0,
            height_ / 2,
            width_,
            height_ - height_ / 2,
            {60, 60, 60}
        );

        // Стены поверх фона.
        raycaster.render(frame, map, player);

        // Вывод полностью готового кадра.
        if (!presenter.present(frame)) {
            std::cerr << "Presentation failed: "
                      << SDL_GetError() << "\n";
            return 1;
        }

        SDL_Delay(1);
    }

    return 0;
}

}
