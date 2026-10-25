#include <cstring>
#include <format>
#include <render/SdlPresenter.h>

void render::SdlPresenter::SDL_Renderer_Deleter::operator()(SDL_Renderer *renderer) const noexcept {
    SDL_DestroyRenderer(renderer);
}

void render::SdlPresenter::SDL_Texture_Deleter::operator()(SDL_Texture *texture) const noexcept {
    SDL_DestroyTexture(texture);
}

render::SdlPresenter::SdlPresenter(SDL_Window *window) {
    if (!window) { throw std::invalid_argument("window cannot be null"); }

    int width = 0, height = 0;
    if (!SDL_GetWindowSizeInPixels(window, &width, &height)) {
        throw std::runtime_error(std::format("failed to get window size: {}", SDL_GetError()));
    }

    if (width == 0 || height == 0) {
        throw std::runtime_error(std::format("invalid window size: {} * {}", width, height));
    }

    width_ = static_cast<buf_size_t>(width);
    height_ = static_cast<buf_size_t>(height);

    if (width_ > std::numeric_limits<buf_size_t>::max() / height_) {
        throw std::length_error("failed to create presenter: Buffer dimensions are too large");
    }

    SDL_Renderer *raw_renderer = SDL_CreateRenderer(window, nullptr);
    if (!raw_renderer) {
        throw std::runtime_error(std::format("failed to create renderer: {}", SDL_GetError()));
    }
    renderer_.reset(raw_renderer);

    SDL_Texture *raw_texture = SDL_CreateTexture(
        renderer_.get(),
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        height, width);
    if (!raw_texture) {
        throw std::runtime_error(std::format("failed to create texture: {}", SDL_GetError()));
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
    //AFTER HEATED DISCUSSION WAS ALTERED TO CURRENT VERSION
    /*
    // using tiling approach in drawing.
    constexpr buf_size_t BLOCK_SIZE = 16; // 16 * 16 = 256 pixels per block (tile)
    const uint32_t *buf_data_ptr = buffer.raw_data();
    uint8_t *texture_pixels_ptr = static_cast<uint8_t *>(pixels);
    //dividing frame into tiles BLOCK_SIZE x BLOCK_SIZE pixels
    for (size_t block_row = 0; block_row < height_; block_row += BLOCK_SIZE) {
        const size_t max_row = std::min(block_row + BLOCK_SIZE, height_); // to control out of bounds UB

        for (size_t block_column = 0; block_column < width_; block_column += BLOCK_SIZE) {
            const size_t max_column = std::min(block_column + BLOCK_SIZE, width_); // to control out of bounds UB

            for (size_t row = block_row; row < max_row; ++row) {
                for (size_t column = block_column; column < max_column; ++column) {
                    // getting address in texture in row-major manner because optimized writing is more important
                    uint8_t *pixel_address = texture_pixels_ptr + row * pitch + column * sizeof(uint32_t);

                    //writing directly to the VRAM
                    *reinterpret_cast<uint32_t *>(pixel_address) = buf_data_ptr[column * height_ + row];
                }
            }
        }
    }*/

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
        error_message = std::format("failed to present texture: {}", SDL_GetError());
        return false;
    }

    return true;
}

std::string render::SdlPresenter::getError() {
    return error_message;
}
