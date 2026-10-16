//
// Created by Nikita on 17.10.2026.
//
#include "Application.h"

#include <algorithm>
#include <iostream>

app::Application::~Application() {
    shutdown();
}

bool app::Application::initialize(const std::string& mapFilename) {
    if (initialized) {
        return true;
    }

    if (!map.loadFromFile(mapFilename)) {
        std::cerr << "Failed to load map: "
                  << mapFilename << '\n';
        return false;
    }

    if (!map.canOccupy(
            map.getSpawnPosition(),
            player.getRadius()
        )) {
        std::cerr << "Invalid player spawn position\n";
        return false;
    }

    player.setPosition(map.getSpawnPosition());

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL initialization failed: "
                  << SDL_GetError() << '\n';
        return false;
    }

    if (!SDL_CreateWindowAndRenderer(
            "RayCaster",
            1024,
            768,
            0,
            &window,
            &renderer
        )) {
        std::cerr << "Window creation failed: "
                  << SDL_GetError() << '\n';

        shutdown();
        return false;
    }

    input.reset();
    initialized = true;

    return true;
}

void app::Application::run() {
    if (!initialized || running) {
        return;
    }

    running = true;

    Uint64 previousTime = SDL_GetTicksNS();

    while (running) {
        Uint64 currentTime = SDL_GetTicksNS();

        double deltaTime =
            static_cast<double>(currentTime - previousTime)
            / 1'000'000'000.0;

        previousTime = currentTime;

        deltaTime = std::min(deltaTime, 0.05);

        input.beginFrame();
        processEvents();

        if (!running) {
            break;
        }

        update(deltaTime);
        render();

        SDL_Delay(1);
    }
}

void app::Application::processEvents() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            stop();
        }

        if (event.type == SDL_EVENT_KEY_DOWN &&
            event.key.key == SDLK_ESCAPE) {
            stop();
        }

        if (event.type == SDL_EVENT_WINDOW_FOCUS_LOST) {
            input.reset();
        }

        // Подключить, когда будет реализован обработчик ввода:
        // input.handleEvent(event);
    }
}

void app::Application::update(double deltaTime) {
    // Заглушка обновления игрового состояния (сам просил).

    (void)deltaTime;
}

void app::Application::render() {
    // Заглушка: вместо изображения мира выводим однотонный фон (тоже сам просил, уважаемый).
    SDL_SetRenderDrawColor(renderer, 30, 30, 40, 255);
    SDL_RenderClear(renderer);


    SDL_RenderPresent(renderer);
}

void app::Application::shutdown() {
    running = false;
    initialized = false;

    input.reset();

    if (renderer != nullptr) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }

    if (window != nullptr) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    SDL_Quit();
}