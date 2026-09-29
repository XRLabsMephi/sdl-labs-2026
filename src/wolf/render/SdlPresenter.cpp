#include "wolf/render/SdlPresenter.hpp"
#include <stdexcept>
#include <string>

namespace render {
    SdlPresenter::SdlPresenter(int width, int height, const std::string& title)
        : width_(width), height_(height) {
        if (width <= 0 || height <= 0) {
            throw std::invalid_argument("Presenter dimensions must be positive");
        }

        if (!SDL_Init(SDL_INIT_VIDEO)) {
            throw std::runtime_error(std::string("SDL initialization failed: ") + SDL_GetError());
        }

        window_ = SDL_CreateWindow(title.c_str(), width, height, SDL_WINDOW_BORDERLESS || SDL_WINDOW_INPUT_FOCUS);
        if (!window_) {
            const std::string error = SDL_GetError();
            SDL_Quit();
            throw std::runtime_error("Window creation failed: " + error);
        }

        renderer_ = SDL_CreateRenderer(window_, NULL);
        if (!renderer_) {
            const std::string error = SDL_GetError();
            SDL_DestroyWindow(window_);
            SDL_Quit();
            throw std::runtime_error("Renderer creation failed: " + error);
        }

        texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, width, height);
        if (!texture_) {
            const std::string error = SDL_GetError();
            SDL_DestroyRenderer(renderer_);
            SDL_DestroyWindow(window_);
            SDL_Quit();
            throw std::runtime_error("Framebuffer texture creation failed: " + error);
        }

        SDL_SetTextureScaleMode(texture_, SDL_SCALEMODE_NEAREST);
        SDL_SetRenderLogicalPresentation(renderer_, width, height, SDL_LOGICAL_PRESENTATION_LETTERBOX);
    }

    SdlPresenter::~SdlPresenter() {
        SDL_DestroyTexture(texture_);
        SDL_DestroyRenderer(renderer_);
        SDL_DestroyWindow(window_);
        SDL_Quit();
    }

    SDL_Window* SdlPresenter::window() noexcept {
        return window_;
    }

    void SdlPresenter::present(const Framebuffer& framebuffer) {
        if (framebuffer.width() != width_ || framebuffer.height() != height_) {
            throw std::invalid_argument("Framebuffer dimensions do not match the presenter");
        }

        const int pitch = framebuffer.width() * static_cast<int>(sizeof(std::uint32_t));
        if (!SDL_UpdateTexture(texture_, nullptr, framebuffer.color(), pitch) || !SDL_RenderClear(renderer_)
            || !SDL_RenderTexture(renderer_, texture_, nullptr, nullptr) || !SDL_RenderPresent(renderer_)) {
            throw std::runtime_error(std::string("Rendering failed: ") + SDL_GetError());
        }
    }
}
