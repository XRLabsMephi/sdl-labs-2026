#include "app/Application.hpp"

Application::Application() {
    if (!map.load_from_file(std::string(ASSETS_DIR) + "/wolf/world/map.txt")) {
        throw std::runtime_error("Map load failed");
    }
    constexpr size_t width = 800;
    constexpr size_t height = 600;
    try {
        presenter = std::make_unique<SDLPresenter>("Raycaster 3D", width, height);
        buffer = std::make_unique<FrameBuffer>(width, height);
        PlayerController::resolve_spawn(player, map);
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Initialization failed: ") + e.what());
    }
}

void Application::run() {
    if (!presenter || !buffer) return;
    Uint64 last = SDL_GetPerformanceCounter();
    Uint64 freq = SDL_GetPerformanceFrequency();
    bool running = true;
    while (running) {
        Uint64 now = SDL_GetPerformanceCounter();
        double dt = static_cast<double>(now - last) / static_cast<double>(freq);
        last = now;
        if (dt > 0.1) dt = 0.1;

        InputState event_input;
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            handle_event(event_input, e);
        }

        InputState input = read_input(SDL_GetKeyboardState(nullptr));
        input.quit = event_input.quit;

        if (input.quit) running = false;
        controller.update(player, input, map, dt);
        buffer->clear();
        raycaster.render(player, map, *buffer);
        presenter->present(*buffer);
        SDL_Delay(1);
    }
}