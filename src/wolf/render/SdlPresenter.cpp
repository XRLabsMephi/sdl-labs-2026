#include <cstring>
#include <format>
#include <render/SdlPresenter.h>

#include "debug/Logger.h"

namespace {
    constexpr const char* component = "PRSNTR";
}

void render::SdlPresenter::SDL_Renderer_Deleter::operator()(SDL_Renderer *renderer) const noexcept {
    SDL_DestroyRenderer(renderer);
}

void render::SdlPresenter::SDL_Texture_Deleter::operator()(SDL_Texture *texture) const noexcept {
    SDL_DestroyTexture(texture);
}

void render::SdlPresenter::SDL_Window_Deleter::operator()(SDL_Window *window) const noexcept {
    SDL_DestroyWindow(window);
}

void render::SdlPresenter::SDL_Surface_Deleter::operator()(SDL_Surface *surface) const noexcept {
    SDL_DestroySurface(surface);
}

void render::SdlPresenter::fail(std::string_view message) {
    error_message = message;
}

render::SdlPresenter::SdlPresenter(SDL_Window *window) {
    if (!window) { throw std::invalid_argument("window cannot be null"); }

    int width = 0, height = 0;
    if (!SDL_GetWindowSizeInPixels(window, &width, &height)) {
        fail(std::format("failed to get window size: {}", SDL_GetError()));
        throw std::runtime_error(error_message);
    }

    window_.reset(window);

    if (width == 0 || height == 0) {
        fail(std::format("invalid window size: {} * {}", width, height));
        throw std::runtime_error(error_message);
    }

    width_ = static_cast<buf_size_t>(width);
    height_ = static_cast<buf_size_t>(height);

    if (width_ > std::numeric_limits<buf_size_t>::max() / height_) {
        fail("failed to create presenter: Buffer dimensions are too large");
        throw std::length_error(error_message);
    }

    SDL_Surface* surface = SDL_GetWindowSurface(window);
    if (!surface) {
        fail(std::format("failed to get window surface: {}", SDL_GetError()));
        throw std::runtime_error(error_message);
    }

    surface_.reset(surface);

    /*SDL_Renderer *raw_renderer = SDL_CreateRenderer(window, SDL_GPU_RENDERER);
    if (!raw_renderer) {
        fail(std::format("failed to create renderer: {}", SDL_GetError()));
        throw std::runtime_error(error_message);
    }
    renderer_.reset(raw_renderer);

    SDL_Texture *raw_texture = SDL_CreateTexture(
        renderer_.get(),
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        height, width);
    if (!raw_texture) {
        fail(std::format("failed to create texture: {}", SDL_GetError()));
        throw std::runtime_error(error_message);
    }
    texture_.reset(raw_texture);*/
}

bool render::SdlPresenter::present(const FrameBuffer &buffer) {
    if (buffer.width() != width_ || buffer.height() != height_) {
        error_message = std::format(
            "buffer dimensions ({} x {}) do not match presenter dimensions ({} x {})",
            buffer.width(), buffer.height(),
            width_, height_);
        return false;
    }

    if (!SDL_LockSurface(surface_.get())) {
        fail(std::format("failed to lock surface: {}", SDL_GetError()));
        return false;
    }

    const Uint32 * __restrict src = buffer.raw_data();
    auto * __restrict dst = static_cast<uint32_t *>(surface_->pixels);
    const size_t stride = surface_->pitch / sizeof(uint32_t);

    constexpr size_t BLOCK_SIZE = 32;
    for (size_t y = 0; y < height_; y += BLOCK_SIZE) {
        const size_t my = std::min(y + BLOCK_SIZE, height_);
        for (size_t x = 0; x < width_; x += BLOCK_SIZE) {
            const size_t mx = std::min(x + BLOCK_SIZE, width_);

            for (size_t j = y; j < my; ++j) {
                Uint32 * __restrict dst_row = dst + j * stride + x;
                const Uint32 * src_col = src + x * height_ + j;
                for (size_t i = 0; i < mx - x; ++i) {
                    dst_row[i] = src_col[i * height_];
                }
            }
        }
    }

    SDL_UnlockSurface(surface_.get());

    if (!SDL_UpdateWindowSurface(window_.get())) {
        fail(std::format("failed to update window surface: {}", SDL_GetError()));
        return false;
    }


    /*void* pixels = nullptr;
    int pitch = 0;
    if (!SDL_LockTexture(texture_.get(), nullptr, &pixels, &pitch)) {
        error_message = std::format("failed to lock texture: {}", SDL_GetError());
        return false;
    }

    // GPU-transposed-flipped rendering: setting up texture as height_ x width_ and transforming it in query to GPU
    const size_t bytes_per_row = height_ * sizeof(uint32_t);
    const auto *src_buffer = reinterpret_cast<const uint8_t *>(buffer.raw_data());
    auto *dst_texture = static_cast<uint8_t *>(pixels);

    if (static_cast<size_t>(pitch) == bytes_per_row) {
        std::memcpy(dst_texture, src_buffer, bytes_per_row * width_);
    } else {
        for (size_t column = 0; column < width_; ++column) {
            std::memcpy(dst_texture + column * pitch, src_buffer + column * bytes_per_row, bytes_per_row);
        }
    }

    SDL_UnlockTexture(texture_.get());
    SDL_RenderTextureRotated(
        renderer_.get(),
        texture_.get(),
        nullptr,
        nullptr,
        90.0,
        nullptr, SDL_FLIP_VERTICAL);
    if (!SDL_RenderPresent(renderer_.get())) {
        fail(std::format("failed to present render: {}", SDL_GetError()));
        return false;
    }*/
    return true;
}

std::string render::SdlPresenter::getError() {
    return error_message;
}
