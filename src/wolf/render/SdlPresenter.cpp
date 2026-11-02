#include "wolf/render/SdlPresenter.hpp"

#include "wolf/render/FrameBuffer.hpp"

#include <SDL3/SDL.h>

#include <stdexcept>
#include <string>

namespace wolf {

SdlPresenter::SdlPresenter(int width, int height) : width_(width), height_(height) {
    try {
        window_ = SDL_CreateWindow("Raycaster - 3passengers", width * 2, height * 2,
                                   SDL_WINDOW_RESIZABLE);
        if (!window_) {
            throw std::runtime_error(SDL_GetError());
        }
        renderer_ = SDL_CreateRenderer(window_, nullptr);
        if (!renderer_) {
            throw std::runtime_error(SDL_GetError());
        }
        texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888,
                                      SDL_TEXTUREACCESS_STREAMING, width, height);
        if (!texture_
            || !SDL_SetTextureScaleMode(texture_, SDL_SCALEMODE_NEAREST)
            || !SDL_SetRenderLogicalPresentation(renderer_, width, height,
                                                 SDL_LOGICAL_PRESENTATION_LETTERBOX)
            || !SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255)) {
            throw std::runtime_error(SDL_GetError());
        }
        SDL_SetRenderVSync(renderer_, 1);
    } catch (...) {
        SDL_DestroyTexture(texture_);
        SDL_DestroyRenderer(renderer_);
        SDL_DestroyWindow(window_);
        throw;
    }
}

SdlPresenter::~SdlPresenter() {
    SDL_DestroyTexture(texture_);
    SDL_DestroyRenderer(renderer_);
    SDL_DestroyWindow(window_);
}

void SdlPresenter::present(const FrameBuffer& frame) {
    if (frame.width() != width_ || frame.height() != height_) {
        throw std::invalid_argument("Размер кадра не совпадает с размером текстуры");
    }
    if (!SDL_UpdateTexture(texture_, nullptr, frame.data(), frame.pitch())
        || !SDL_RenderClear(renderer_)
        || !SDL_RenderTexture(renderer_, texture_, nullptr, nullptr)
        || !SDL_RenderPresent(renderer_)) {
        throw std::runtime_error("Ошибка вывода кадра: " + std::string(SDL_GetError()));
    }
}

}
