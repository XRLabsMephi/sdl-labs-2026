#include "wolf/render/SdlPresenter.hpp"
#include "wolf/render/FrameBuffer.hpp"

#include <SDL3/SDL.h>

#include <cstddef>
#include <cstring>

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

    void* pixels = nullptr;
    int pitch = 0;

    if (!SDL_LockTexture(texture_, nullptr, &pixels, &pitch)) {
        return false;
    }

    const auto rowBytes =
        static_cast<std::size_t>(width_) * sizeof(Color);

    if (!pixels || pitch < 0 ||
        static_cast<std::size_t>(pitch) < rowBytes) {
        SDL_UnlockTexture(texture_);
        return false;
    }

    const auto* source =
        reinterpret_cast<const std::byte*>(frame.data());

    auto* destination = static_cast<std::byte*>(pixels);

    for (int y = 0; y < height_; ++y) {
        std::memcpy(
            destination + static_cast<std::size_t>(y) * pitch,
            source + static_cast<std::size_t>(y) * rowBytes,
            rowBytes
        );
    }

    SDL_UnlockTexture(texture_);

    if (!SDL_RenderClear(renderer_)) {
        return false;
    }

    if (!SDL_RenderTexture(renderer_, texture_, nullptr, nullptr)) {
        return false;
    }

    return SDL_RenderPresent(renderer_);
}

}
