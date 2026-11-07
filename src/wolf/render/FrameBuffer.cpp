#include <render/FrameBuffer.h>
#include <stdexcept>
#include <ranges>
#include <algorithm>
#include <limits>

#include "render/Texture.h"

using namespace render;
using std::min;
using std::max;

FrameBuffer::FrameBuffer(int width, int height)
    : width_(0)
      , height_(0)
      , int_width_(0)
      , int_height_(0) {
    if (width <= 0) { throw std::invalid_argument("width must be > 0"); }
    if (height <= 0) { throw std::invalid_argument("height must be > 0"); }

    width_ = static_cast<buf_size_t>(width);
    height_ = static_cast<buf_size_t>(height);

    if (width_ > std::numeric_limits<buf_size_t>::max() / height_) {
        throw std::length_error("Buffer dimensions are too large");
    }

    int_width_ = width;
    int_height_ = height;

    buffer_.assign(width_ * height_, 0);
}

void FrameBuffer::set_fog_params(Color fog_color, float min_fog_depth, float max_fog_depth) {
    fog_ = Fog(min_fog_depth, max_fog_depth, fog_color);
}

buf_size_t FrameBuffer::width() const { return width_; }

buf_size_t FrameBuffer::height() const { return height_; }

const uint32_t *FrameBuffer::raw_data() const { return buffer_.data(); }

void FrameBuffer::clear(const Color color) {
    std::ranges::fill(buffer_, static_cast<Uint32>(color));
}

void FrameBuffer::clear(const DrawCallParams &params) {
    //TODO
}

void FrameBuffer::set(int x, int y, const Color color) {
    if (x < 0 || y < 0 || x >= int_width_ || y >= int_height_) return;

    buffer_[x * height_ + y] = static_cast<Uint32>(color);
}

void FrameBuffer::set(int x, int y, const DrawCallParams &params) {
    if (x < 0 || y < 0 || x >= int_width_ || y >= int_height_) return;

    buffer_[x * height_ + y] = params.tex_col[y]; //TODO пересчитать через step
}

void FrameBuffer::vline(int x, int y0, int y1, const Color color) {
    if (x < 0 || x >= int_width_) return;
    if (y0 > y1) std::swap(y0, y1);

    y0 = max(y0, 0);
    y1 = min(y1, int_height_ - 1);
    if (y0 > y1) return;

    uint32_t *col = &buffer_[x * height_];
    std::fill_n(col + y0, y1 - y0 + 1, static_cast<Uint32>(color));
}

template<bool IsSprite>
void FrameBuffer::vline(int x, int y0, int y1, const DrawCallParams &params) {
    if (x < 0 || x >= int_width_) return;

    const auto &[tex_col, tex_y0, step, depth, brightness] = params;
    if (step <= 0 || tex_y0 < 0) return;
    if (y0 > y1) std::swap(y0, y1);

    int clipping_offset = 0;
    if (y0 < 0) { clipping_offset = -y0 * step, y0 = 0; }
    y1 = min(y1, int_height_ - 1);
    if (y0 > y1) return;

    Uint8 fog_factor = fog_.factor(depth);
    Uint32 *col = &buffer_[x * height_];

    const Uint8 *lut = core::DIM[brightness].data();
    //step is fixed precision number in 16.16 format
    for (int y = y0, tex_y = tex_y0 + clipping_offset; y <= y1; ++y, tex_y += step) {
        Uint32 texel = tex_col[tex_y >> 16];

        if constexpr (IsSprite) { if (!(texel & 0xFF000000u)) continue; }

        Uint32 lit_texel = static_cast<Uint32>(Color::fromBits(texel).dimmed(lut));
        Uint32 fogged_texel = fog_.applyFog(lit_texel, fog_factor);
        col[y] = static_cast<Uint32>(Color::fromBits(fogged_texel).over(Color::fromBits(col[y])));
    }
}

void FrameBuffer::hline(const int y, int x0, int x1, const Color color) {
    if (y < 0 || y >= int_height_) return;
    if (x0 > x1) std::swap(x0, x1);
    x0 = max(x0, 0);
    x1 = min(x1, int_width_ - 1);
    if (x0 > x1) return;

    const buf_size_t h = height_;
    const auto x_end = static_cast<buf_size_t>(x1);
    for (buf_size_t x = x0; x <= x_end; ++x) {
        buffer_[x * h + y] = static_cast<Uint32>(color);
    }
}

void FrameBuffer::hline(const int y, int x0, int x1, const DrawCallParams &params) {
    if (y < 0 || y >= int_height_) return;
    if (x0 > x1) std::swap(x0, x1);
    x0 = max(x0, 0);
    x1 = min(x1, int_width_ - 1);
    if (x0 > x1) return;

    //TODO
}

void FrameBuffer::rectangle(int x0, int y0, int x1, int y1, const Color color) {
    if (y0 > y1) std::swap(y0, y1);
    if (x0 > x1) std::swap(x0, x1);

    y0 = max(y0, 0);
    x0 = max(x0, 0);
    x1 = min(x1, int_width_ - 1);
    y1 = min(y1, int_height_ - 1);

    if (x0 > x1 || y0 > y1) return;

    for (int x = x0; x <= x1; ++x) {
        // TODO Add multithreading
        uint32_t *colPtr = &buffer_[x * height_];
        std::fill_n(colPtr + y0, y1 - y0 + 1, static_cast<Uint32>(color));
    }
}

void FrameBuffer::rectangle(int x0, int y0, int x1, int y1, DrawCallParams &params) {
    if (y0 > y1) std::swap(y0, y1);
    if (x0 > x1) std::swap(x0, x1);

    y0 = max(y0, 0);
    x0 = max(x0, 0);
    x1 = min(x1, int_width_ - 1);
    y1 = min(y1, int_height_ - 1);

    if (x0 > x1 || y0 > y1) return;

    //TODO
}
