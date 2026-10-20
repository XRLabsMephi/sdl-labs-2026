//
// Created by Nikita on 17.10.2026.
//
#include "Application.h"

#include <algorithm>
#include <iostream>

namespace {

    constexpr char WINDOW_TITLE[] = "RayCaster";

    constexpr int WINDOW_WIDTH = 1024;
    constexpr int WINDOW_HEIGHT = 768;

    constexpr SDL_WindowFlags WINDOW_FLAGS = 0;

    constexpr float NANOSECONDS_PER_SECOND = 1'000'000'000.0f;

}

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

    sdlInitialized = true;

    window.reset(
        SDL_CreateWindow(
            WINDOW_TITLE,
            WINDOW_WIDTH,
            WINDOW_HEIGHT,
            WINDOW_FLAGS
        )
    );
    // TODO: Initialize SdlPresenter with window.get(); Application retains window ownership.

    if (!window) {
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

        // Convert elapsed time from nanoseconds to seconds.
        float deltaTime =
    static_cast<float>(currentTime - previousTime)
    / NANOSECONDS_PER_SECOND;

        previousTime = currentTime;

        // Reset per-frame input before processing new events.
        input.beginFrame();
        processEvents();

        // Skip the rest of the frame if an exit was requested.
        if (!running) {
            break;
        }

        update(deltaTime);
        // TODO: Render the scene into FrameBuffer and display it through SdlPresenter.
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

    }
}

void app::Application::update(float deltaTime) {
    // TODO: Update the player through PlayerController using input and deltaTime.
    // TODO: Update other game objects.
    (void)deltaTime;
}

void app::Application::shutdown() {
    running = false;
    initialized = false;

    input.reset();

    // TODO: Release SdlPresenter resources before destroying the window.
    window.reset();

    if (sdlInitialized) {
        // All owned SDL resources must be released before shutting down SDL.
        SDL_Quit();
        sdlInitialized = false;
    }
}