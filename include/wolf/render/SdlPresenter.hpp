#pragma once
#include <string>
#include <SDL3/SDL.h>

#include "wolf/render/framebuffer.hpp"

namespace render {
    class SdlPresenter {
    public:
        SdlPresenter(int width, int height, const std::string& title = "Software Graphics Labs");
        ~SdlPresenter();

        SdlPresenter(const SdlPresenter&) = delete;
        SdlPresenter& operator=(const SdlPresenter&) = delete;
        SdlPresenter(SdlPresenter&&) = delete;
        SdlPresenter& operator=(SdlPresenter&&) = delete;

        [[nodiscard]] SDL_Window* window() noexcept;

        void present(const Framebuffer& framebuffer);
    private:
        SDL_Window* window_ = nullptr;
        SDL_Renderer* renderer_ = nullptr;
        SDL_Texture* texture_ = nullptr;
        int width_ = 0;
        int height_ = 0;
    };
}
