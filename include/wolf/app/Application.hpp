#pragma once

struct SDL_Window;
struct SDL_Renderer;

namespace wolf {

    class Application {
    public:
        Application() = default;
        ~Application();

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        int run();

    private:
        bool init();
        int run_loop();

        static constexpr int width_ = 1600;
        static constexpr int height_ = 1200;

        SDL_Window* window_ = nullptr;
        SDL_Renderer* renderer_ = nullptr;

        bool sdl_initialized_ = false;
    };

}
