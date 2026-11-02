#include "wolf/app/Application.hpp"

#include "wolf/input/PlayerController.hpp"
#include "wolf/raycast/Raycaster.hpp"
#include "wolf/render/FrameBuffer.hpp"
#include "wolf/render/SdlPresenter.hpp"
#include "wolf/texture/TextureStore.hpp"
#include "wolf/world/Map.hpp"
#include "wolf/world/Player.hpp"

#include <SDL3/SDL.h>

#include <algorithm>
#include <exception>
#include <filesystem>
#include <iostream>
#include <stdexcept>

namespace wolf {

InputState Application::readInput() const {
    InputState input;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
            input.quit = true;
        }
    }
    const bool* keys = SDL_GetKeyboardState(nullptr);
    input.quit = input.quit || keys[SDL_SCANCODE_ESCAPE];
    if (SDL_GetKeyboardFocus()) {
        input.forward = keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP];
        input.backward = keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN];
        input.strafeLeft = keys[SDL_SCANCODE_A];
        input.strafeRight = keys[SDL_SCANCODE_D];
        input.turnLeft = keys[SDL_SCANCODE_LEFT];
        input.turnRight = keys[SDL_SCANCODE_RIGHT];
    }
    return input;
}

int Application::run() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "Ошибка SDL: " << SDL_GetError() << '\n';
        return 1;
    }
    int result = 0;
    try {
        const char* base = SDL_GetBasePath();
        if (!base) {
            throw std::runtime_error(SDL_GetError());
        }
        const auto assets = std::filesystem::path(base) / "assets/wolf";
        const Map map = Map::load(assets / "maps/map01.txt");
        TextureStore textures;
        for (int id = 1; id <= 4; ++id) {
            textures.load(id, assets / "textures" / ("wall0" + std::to_string(id) + ".bmp"));
        }
        Player player;
        PlayerController controller;
        if (!controller.canOccupy(map, player.position)) {
            throw std::runtime_error("Начальная позиция игрока находится в стене");
        }
        FrameBuffer frame(640, 400);
        SdlPresenter presenter(frame.width(), frame.height());
        Raycaster raycaster;
        Uint64 previous = SDL_GetTicksNS();
        while (true) {
            const Uint64 start = SDL_GetTicksNS();
            const double dt = std::min((start - previous) / 1e9, 0.1);
            previous = start;
            const InputState input = readInput();
            if (input.quit) {
                break;
            }
            controller.update(player, map, input, dt);
            raycaster.render(map, player, frame, &textures);
            presenter.present(frame);
            constexpr Uint64 frameTime = 1000000000 / 120;
            const Uint64 elapsed = SDL_GetTicksNS() - start;
            if (elapsed < frameTime) {
                SDL_DelayNS(frameTime - elapsed);
            }
        }
    } catch (const std::exception& error) {
        std::cerr << "Ошибка: " << error.what() << '\n';
        result = 1;
    }
    SDL_Quit();
    return result;
}

}
