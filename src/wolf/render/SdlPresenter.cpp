#include "wolf/render/SdlPresenter.hpp"
#include "wolf/render/FrameBuffer.hpp"

#include <stdexcept>

SdlPresenter::SdlPresenter(int width, int height) : width_(width), height_(height) {
    if (width <= 0 || height <= 0) {
        throw std::invalid_argument("Window dimensions must be positive");
    }
    try {
        if (!SDL_CreateWindowAndRenderer("Wolf Lab 1 | WASD: move | Arrows: turn | Esc: exit", width_, height_, 0, &window_, &renderer_)) {
            throw std::runtime_error(SDL_GetError());
        }
        texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width_, height_);
        if (!texture_) {
            throw std::runtime_error(SDL_GetError());
        }
        if (!SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_NONE)) {
            throw std::runtime_error(SDL_GetError());
        }
        SDL_SetRenderVSync(renderer_, 1);
    } catch (...) {
        release();
        throw;
    }
}

SdlPresenter::~SdlPresenter() {
    release();
}

void SdlPresenter::release() {
    if (texture_) {
        SDL_DestroyTexture(texture_);
        texture_ = nullptr;
    }
    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }
    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
}

void SdlPresenter::present(const FrameBuffer& frame) {
    if (frame.width() != width_ || frame.height() != height_) {
        throw std::invalid_argument("FrameBuffer size does not match texture size");
    }
    int pitch = frame.width() * static_cast<int>(sizeof(Color));
    if (!SDL_UpdateTexture(texture_, nullptr, frame.data(), pitch)) {
        throw std::runtime_error(SDL_GetError());
    }
    if (!SDL_RenderClear(renderer_)) {
        throw std::runtime_error(SDL_GetError());
    }
    if (!SDL_RenderTexture(renderer_, texture_, nullptr, nullptr)) {
        throw std::runtime_error(SDL_GetError());
    }
    if (!SDL_RenderPresent(renderer_)) {
        throw std::runtime_error(SDL_GetError());
    }
}