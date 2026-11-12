#include "wolf/render/SdlPresenter.hpp"

#include <format>
#include <stdexcept>

namespace wolf::render {

    SdlPresenter::SdlPresenter(SDL_Renderer* renderer, int width, int height) : renderer(renderer) {
        if (!renderer) {
            throw std::runtime_error(std::format("Could not create renderer: {}", SDL_GetError()));
        }
        if (width <= 0 || height <= 0) {
            throw std::runtime_error(std::format("Invalid width or height: {}", width, height));
        }
        texture = SDL_CreateTexture( renderer,
            SDL_PIXELFORMAT_RGBA32,
            SDL_TEXTUREACCESS_STREAMING,
            width,height
            );
        if (!texture) {
            throw std::runtime_error(std::format("Could not create texture: {}", SDL_GetError()));
        }
    }

    SdlPresenter::~SdlPresenter() {
        if (texture) {
            SDL_DestroyTexture(texture);
        }
    }

    void SdlPresenter::present(const FrameBuffer& frameBuffer) {
        //пиксели из CPU FrameBuffer в текстуру GPU
        const int width = frameBuffer.getWidth();
        if (width <= 0) {
            throw std::runtime_error(std::format("Invalid width: {}", width));
        }
        int pitch = width * static_cast<int>(sizeof(std::uint32_t));
        if (!SDL_UpdateTexture(texture, nullptr, frameBuffer.getPixels(), pitch)) {
            throw std::runtime_error(std::format("Could not update texture: {}", SDL_GetError()));
        }
        if (!(SDL_RenderClear(renderer) || SDL_RenderTexture(renderer, texture, nullptr, nullptr) ||
            SDL_RenderPresent(renderer))) {
            throw std::runtime_error(std::format("Could not present texture: {}", SDL_GetError()));
        }
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }
}
