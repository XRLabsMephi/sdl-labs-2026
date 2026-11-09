#include <format>
#include <iostream>
#include <render/SdlPresenter.h>

#include "debug/Logger.h"

namespace {
    constexpr const char *component = "PRSNTR";
}

void render::SdlPresenter::SDL_Window_Deleter::operator()(SDL_Window *window) const noexcept {
    SDL_DestroyWindow(window);
}

void render::SdlPresenter::SDL_Surface_Deleter::operator()(SDL_Surface *surface) const noexcept {
    SDL_DestroySurface(surface);
}

void render::SdlPresenter::fail(std::string_view message) {
    error_message = message;
    logging::error(component, 2, error_message, "");
}

render::SdlPresenter::SdlPresenter(SDL_Window *window) {
    if (!window) { throw std::invalid_argument("window cannot be null"); }

    window_.reset(window);

    int width = 0, height = 0;
    if (!SDL_GetWindowSizeInPixels(window, &width, &height)) {
        throw std::runtime_error(std::format("failed to get window size: {}", SDL_GetError()));
    }

    SDL_Surface *surface = SDL_GetWindowSurface(window_.get());
    if (!surface) {
        fail(std::format("failed to get window surface: {}", SDL_GetError()));
        throw std::runtime_error(error_message);
    }

    surface_.reset(surface);


    if (width == 0 || height == 0) {
        throw std::runtime_error(std::format("invalid window size: {} * {}", width, height));
    }

    width_ = static_cast<buf_size_t>(width);
    height_ = static_cast<buf_size_t>(height);

    if (width_ > std::numeric_limits<buf_size_t>::max() / height_) {
        throw std::length_error("failed to create presenter: Buffer dimensions are too large");
    }
}

bool render::SdlPresenter::present(const FrameBuffer &buffer) {

    if (buffer.width() != width_ || buffer.height() != height_) {
        fail(std::format("dimensions of buffer don't match with presenter dimensions"));
        return false;
    }

    if (!SDL_LockSurface(surface_.get())) {
        fail(std::format("failed to lock surface: {}", SDL_GetError()));
        return false;
    }

    const Uint32 * __restrict src = buffer.raw_data();
    auto * __restrict dst = static_cast<uint32_t *>(surface_->pixels);
    const int stride = static_cast<int>(surface_->pitch / sizeof(uint32_t));

    constexpr size_t BLOCK_SIZE = 32;
    for (size_t y = 0; y < height_; y += BLOCK_SIZE) {
        for (size_t x = 0; x < width_; x += BLOCK_SIZE) {
            size_t mx = std::min(x + BLOCK_SIZE, width_);
            size_t my = std::min(y + BLOCK_SIZE, height_);

            for (size_t j = y; j < my; ++j) {
                for (size_t i = x; i < mx; ++i) {
                    dst[j * stride + i] = src[i * height_ + j];
                }
            }
        }
    }

    SDL_UnlockSurface(surface_.get());

    if (!SDL_UpdateWindowSurface(window_.get())) {
        fail(std::format("failed to update window surface: {}", SDL_GetError()));
        return false;
    }

    return true;
}

std::string render::SdlPresenter::getError() {
    return error_message;
}
