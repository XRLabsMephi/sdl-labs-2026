#include <cstring>
#include <format>
#include <render/SdlPresenter.h>

#include "debug/Logger.h"

namespace {
    constexpr const char *component = "PRSNTR";
}

void render::SdlPresenter::SDL_Renderer_Deleter::operator()(SDL_Renderer *renderer) const noexcept {
    SDL_DestroyRenderer(renderer);
}

void render::SdlPresenter::SDL_Texture_Deleter::operator()(SDL_Texture *texture) const noexcept {
    SDL_DestroyTexture(texture);
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

    SDL_Renderer *raw_renderer = SDL_CreateRenderer(window, nullptr);
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
    texture_.reset(raw_texture);
}

bool render::SdlPresenter::present(const FrameBuffer &buffer) {
    if (buffer.width() != width_ || buffer.height() != height_) {
        error_message = std::format(
            "buffer dimensions ({} x {}) do not match presenter dimensions ({} x {})",
            buffer.width(), buffer.height(),
            width_, height_);
        return false;
    }

    void *pixels = nullptr;
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

    int out_width, out_height;
    if (!SDL_GetRenderOutputSize(renderer_.get(), &out_width, &out_height)) {
        error_message = std::format("failed to get render output size: {}", SDL_GetError());
        return false;
    }

    const SDL_FRect dst{
        static_cast<float>(out_width - out_height) / 2.0f,
        static_cast<float>(out_height - out_width) / 2.0f,
        static_cast<float>(out_height), static_cast<float>(out_width)
    };

    if (!SDL_RenderTextureRotated(
        renderer_.get(),
        texture_.get(),
        nullptr,
        &dst,
        90.0,
        nullptr,
        SDL_FLIP_VERTICAL)) {
        error_message = std::format("failed to render the texture: {}", SDL_GetError());
        return false;
    }

    if (!SDL_RenderPresent(renderer_.get())) {
        fail(std::format("failed to present render: {}", SDL_GetError()));
        return false;
    }
    return true;
}

std::string render::SdlPresenter::getError() {
    return error_message;
}
