#include "wolf/app/Application.hpp"
#include "wolf/texture/TextureStore.hpp"
#include "wolf/input/InputState.hpp"
#include "wolf/input/PlayerController.hpp"
#include "wolf/raycast/Raycaster.hpp"
#include "wolf/render/FrameBuffer.hpp"
#include "wolf/render/SdlPresenter.hpp"
#include "wolf/world/Map.hpp"
#include "wolf/world/Player.hpp"

#include <SDL3/SDL.h>
#include <iostream>
#include <stdexcept>
#include <string>

int Application::run() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << SDL_GetError() << '\n';
        return 1;
    }

    int result = 0;

    try {
        const char* basePath = SDL_GetBasePath();
        if (!basePath) throw std::runtime_error(SDL_GetError());
        std::string mapPath = std::string(basePath) + "resources/map.txt";
        Map map;
        if (!map.loadFromFile(mapPath)) {
            throw std::runtime_error("Failed to load map: " + mapPath);
        }
        Player player;
        if (map.isWall(1, 1)) throw std::runtime_error("Player start cell must be empty");
        TextureStore textures;
        for (int id = 1; id <= 4; id++) {
            std::string path = std::string(basePath) + "resources/wall" + std::to_string(id) + ".bmp";
            if (!textures.load(id, path)) {
                throw std::runtime_error("Failed to load texture: " + path);
            }
        }
        FrameBuffer frame(800, 600);
        SdlPresenter presenter(frame.width(), frame.height());
        Raycaster raycaster;
        PlayerController controller;
        bool running = true;
        Uint64 previousTime = SDL_GetTicksNS();
        while (running) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) running = false;
            }
            const bool* keys = SDL_GetKeyboardState(nullptr);
            if (keys[SDL_SCANCODE_ESCAPE]) running = false;
            if (!running) break;
            Uint64 currentTime = SDL_GetTicksNS();
            double deltaTime = static_cast<double>(currentTime - previousTime) / 1000000000.0;
            previousTime = currentTime;
            InputState input;
            input.forward = keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP];
            input.backward = keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN];
            input.left = keys[SDL_SCANCODE_A];
            input.right = keys[SDL_SCANCODE_D];
            input.rotateLeft = keys[SDL_SCANCODE_LEFT];
            input.rotateRight = keys[SDL_SCANCODE_RIGHT];
            controller.update(player, map, input, deltaTime);
            raycaster.render(map, player, frame, textures);
            presenter.present(frame);
            SDL_Delay(1);
        }
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        result = 1;
    }
    SDL_Quit();
    return result;
}