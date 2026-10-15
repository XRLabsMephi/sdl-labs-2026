#include "wolf/render/SDLPresenter.hpp"
#include <stdexcept>

SDLPresenter::SDLPresenter(const std::string& title, size_t width, size_t height)
    : width_(width), height_(height) {

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        throw std::runtime_error("Failed to initialize SDL3: " + std::string(SDL_GetError()));
    }

    if (!SDL_CreateWindowAndRenderer(title.c_str(), static_cast<int>(width_), static_cast<int>(height_), 0, &window_, &renderer_)) {
        SDL_Quit();
        throw std::runtime_error("Failed to create SDL Window/Renderer: " + std::string(SDL_GetError()));
    }

    texture_ = SDL_CreateTexture(
        renderer_,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        static_cast<int>(width_),
        static_cast<int>(height_)
    );

    if (!texture_) {
        SDL_DestroyRenderer(renderer_);
        SDL_DestroyWindow(window_);
        SDL_Quit();
        throw std::runtime_error("Failed to create SDL Texture: " + std::string(SDL_GetError()));
    }
}

SDLPresenter::~SDLPresenter() {
    if (texture_) SDL_DestroyTexture(texture_);
    if (renderer_) SDL_DestroyRenderer(renderer_);
    if (window_) SDL_DestroyWindow(window_);
    SDL_Quit();
}

void SDLPresenter::present(const FrameBuffer& buffer) {
    int pitch = static_cast<int>(buffer.get_width() * sizeof(Color));

    SDL_UpdateTexture(
        texture_,
        nullptr,
        buffer.get_pixels(),
        pitch
    );

    SDL_RenderClear(renderer_);
    SDL_RenderTexture(renderer_, texture_, nullptr, nullptr);
    SDL_RenderPresent(renderer_);
}