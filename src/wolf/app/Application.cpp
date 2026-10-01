#include "app/Application.hpp"

Application::Application() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return;
    }

    window_ = SDL_CreateWindow("Raycaster", 800, 600, 0);
    if (!window_) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
        return;
    }

    if (!map.load_from_file(std::string(ASSETS_DIR) + "/wolf/world/map.txt")) {
        std::cerr << "map load failed\n";
    }
}

void Application::run() {
    Uint64 last = SDL_GetPerformanceCounter();
    Uint64 freq = SDL_GetPerformanceFrequency();
    bool running = true;

    while (running) {
        Uint64 now = SDL_GetPerformanceCounter();
        double dt = static_cast<double>(now - last) / static_cast<double>(freq);
        last = now;

        if (dt > 0.1) dt = 0.1;

        InputState input = read_input(SDL_GetKeyboardState(nullptr));

        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            handle_event(input, e);
        }

        if (input.quit) running = false;
        controller.update(player, input, map, dt);

        std::cout << "pos=(" << player.position.x
                  << ", " << player.position.y << ")"
                  << "  dir=(" << player.direction.x
                  << ", " << player.direction.y << ")\n";

        SDL_Delay(16);
    }
}

Application::~Application() {
    if (window_) SDL_DestroyWindow(window_);
    SDL_Quit();
}