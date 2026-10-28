#include "wolf/render/SdlPresenter.hpp"

#include "wolf/render/FrameBuffer.hpp"

#include <SDL3/SDL.h>

#include <limits>
#include <stdexcept>
#include <string>

namespace wolf {
namespace {

[[nodiscard]] std::runtime_error sdlError(const char* operation) {
    return std::runtime_error(std::string(operation) + ": " + SDL_GetError());
}

} // namespace

SdlPresenter::SdlPresenter(SDL_Window& window, int width, int height)
    : width_(width), height_(height) {
    if (width <= 0 || height <= 0 ||
        width > std::numeric_limits<int>::max() / static_cast<int>(sizeof(Color))) {
        throw std::invalid_argument("Invalid SdlPresenter dimensions");
    }
    renderer_ = SDL_CreateRenderer(&window, nullptr);
    if (!renderer_) { throw sdlError("SDL_CreateRenderer"); }

    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA8888,
                                 SDL_TEXTUREACCESS_STREAMING, width_, height_);
    if (!texture_ ||
        !SDL_SetTextureScaleMode(texture_, SDL_SCALEMODE_NEAREST) ||
        !SDL_SetRenderLogicalPresentation(renderer_, width_, height_,
                                          SDL_LOGICAL_PRESENTATION_LETTERBOX)) {
        const auto error = sdlError("SdlPresenter setup");
        SDL_DestroyTexture(texture_);
        SDL_DestroyRenderer(renderer_);
        texture_ = nullptr;
        renderer_ = nullptr;
        throw error;
    }
}

SdlPresenter::~SdlPresenter() {
    SDL_DestroyTexture(texture_);
    SDL_DestroyRenderer(renderer_);
}

void SdlPresenter::present(const FrameBuffer& frame) {
    if (frame.width() != width_ || frame.height() != height_) {
        throw std::invalid_argument("SdlPresenter and FrameBuffer dimensions differ");
    }
    const int pitch = width_ * static_cast<int>(sizeof(Color));
    if (!SDL_UpdateTexture(texture_, nullptr, frame.pixels(), pitch)) {
        throw sdlError("SDL_UpdateTexture");
    }
    if (!SDL_RenderClear(renderer_)) { throw sdlError("SDL_RenderClear"); }
    if (!SDL_RenderTexture(renderer_, texture_, nullptr, nullptr)) {
        throw sdlError("SDL_RenderTexture");
    }
    if (!SDL_RenderPresent(renderer_)) { throw sdlError("SDL_RenderPresent"); }
}

} // namespace wolf
