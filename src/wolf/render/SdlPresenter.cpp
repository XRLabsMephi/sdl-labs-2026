#include "wolf/render/SdlPresenter.hpp"
#include "wolf/render/FrameBuffer.hpp"

#include <SDL3/SDL.h>

namespace wolf {

SdlPresenter::SdlPresenter(SDL_Renderer* renderer, int width, int height)
    : renderer_(renderer), width_(width), height_(height) {
    texture_ = SDL_CreateTexture(
        renderer_,
        SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STREAMING,
        width_,
        height_
    );

    if (texture_ &&
        !SDL_SetTextureScaleMode(texture_, SDL_SCALEMODE_NEAREST)) {
        SDL_DestroyTexture(texture_);
        texture_ = nullptr;
    }
}

SdlPresenter::~SdlPresenter() {
    if (texture_) SDL_DestroyTexture(texture_);
}

bool SdlPresenter::present(const FrameBuffer& frame) {
    if (!texture_ || frame.width() != width_ || frame.height() != height_) {
        return false;
    }

    int pitch = width_ * sizeof(Color);

    if (!SDL_UpdateTexture(texture_, nullptr, frame.data(), pitch)) {
        return false;
    }

    if (!SDL_RenderClear(renderer_)) {
        return false;
    }

    if (!SDL_RenderTexture(renderer_, texture_, nullptr, nullptr)) {
        return false;
    }

    return SDL_RenderPresent(renderer_);
}

}
