#include <render/FrameBuffer.h>
#include <stdexcept>
#include <ranges>
#include <algorithm>
#include <limits>

#include "render/Texture.h"

using namespace render;
using std::min;
using std::max;

FrameBuffer::FrameBuffer(int width, int height, TextureStorage &storage)
    : width_(0)
      , height_(0)
      , int_width_(0)
      , int_height_(0)
      , storage_(storage) {
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
    zbuf_.assign(width_, std::numeric_limits<float>::infinity());
}

void FrameBuffer::set_fog_params(Color fog_color, float min_fog_depth, float max_fog_depth) {
    fog_color_ = fog_color.toUint32_t();
    fog_min_depth_ = min_fog_depth;
    fog_max_depth = max_fog_depth;
}

buf_size_t FrameBuffer::width() const { return width_; }

buf_size_t FrameBuffer::height() const { return height_; }

const uint32_t *FrameBuffer::raw_data() const { return buffer_.data(); }

void FrameBuffer::clear(const Color color) {
    const uint32_t color_as_uint = color.toUint32_t();
    std::ranges::fill(buffer_, color_as_uint);
    std::ranges::fill(zbuf_, std::numeric_limits<float>::infinity());
}

void FrameBuffer::clear(const DrawCallParams &params) {
    //TODO
}

void FrameBuffer::set(int x, int y, const Color color) {
    if (x < 0 || y < 0 || x >= int_width_ || y >= int_height_) return;

    const uint32_t color_as_uint = color.toUint32_t();

    buffer_[x * height_ + y] = color_as_uint;
}

void FrameBuffer::set(int x, int y, const DrawCallParams &params) {
    if (x < 0 || y < 0 || x >= int_width_ || y >= int_height_) return;

    buffer_[x * height_ + y] = storage_.get(params.tex_id).pixel(params.tex_x, y);
}

void FrameBuffer::vline(int x, int y0, int y1, const Color color) {
    if (x < 0 || x >= int_width_) return;
    if (y0 > y1) std::swap(y0, y1);

    y0 = max(y0, 0);
    y1 = min(y1, int_height_ - 1);
    if (y0 > y1) return;

    const uint32_t color_as_uint = color.toUint32_t();

    uint32_t *col = &buffer_[x * height_];
    std::fill_n(col + y0, y1 - y0 + 1, color_as_uint);
}

namespace {
    Uint32 apply_brightness(const Uint8 *lut, Uint32 p) {
        return (p & 0xFF000000)
               | static_cast<Uint32>(lut[p >> 16 & 0xFF]) << 16
               | static_cast<Uint32>(lut[p >> 8 & 0xFF]) << 8
               | static_cast<Uint32>(lut[p & 0xFF]);
    }

    Uint32 blend_argb(Uint32 src, Uint32 dst, Uint8 a) {
        // a = 0 -> dst, a = 255 -> src
        const Uint32 ia = 255 - a;
        Uint32 r = (ia * ((dst >> 16) & 0xFF) + a * ((src >> 16) & 0xFF) + 127) / 255;
        Uint32 g = (ia * ((dst >> 8) & 0xFF) + a * ((src >> 8) & 0xFF) + 127) / 255;
        Uint32 b = (ia * (dst & 0xFF) + a * (src & 0xFF) + 127) / 255;
        return 0xFF000000 | (r << 16) | (g << 8) | b;
    }
}

Uint8 FrameBuffer::calc_fog_factor(float depth) const {
    if (depth >= fog_max_depth) { return 255; }
    if (depth > fog_min_depth_) {
        return static_cast<Uint8>(255.0 * (depth - fog_min_depth_) / (fog_max_depth - fog_min_depth_));
    }
    return 0;
}

void FrameBuffer::vline(int x, int y0, int y1, const DrawCallParams &params) {
    if (x < 0 || x >= int_width_) return;
    if (params.depth >= zbuf_[x]) return;

    const auto &[id, tex_x, tex_y0, step,
        depth, brightness,alpha] = params;
    if (step <= 0 || tex_y0 < 0 || alpha == 0) return;
    if (y0 > y1) std::swap(y0, y1);

    int clipping_offset = 0;
    if (y0 < 0) { clipping_offset = -y0 * step, y0 = 0; }
    y1 = min(y1, int_height_ - 1);
    if (y0 > y1) return;

    Uint8 fog_factor = calc_fog_factor(depth);

    const Texture &tex = storage_.get(id);
    if (tex_x < 0 || tex_x >= tex.width) return;

    const Uint32 *tex_col = tex.column(tex_x);
    Uint32 *col = &buffer_[x * height_];

    const Uint8 *lut = core::DIM[brightness].data();
    //step is fixed precision number in 16.16 format
    for (int y = y0, tex_y = tex_y0 + clipping_offset; y <= y1; ++y, tex_y += step) {
        int tex_y_clamped = std::clamp(tex_y >> 16, 0, tex.height - 1); // converting 16.16 back to integer

        Uint32 texel = tex_col[tex_y_clamped];

        Uint32 lit_texel = apply_brightness(lut, texel);
        Uint32 fogged_texel = fog_factor != 255 ? blend_argb(fog_color_, lit_texel, fog_factor) : fog_color_;

        if (alpha == 255) {
            col[y] = fogged_texel;
        } else { col[y] = blend_argb(fogged_texel, col[y], alpha); }
    }
    if (alpha == 255)  zbuf_[x] = depth;
}

void FrameBuffer::vline_sprite(int x, int y0, int y1, const DrawCallParams &params) {
    if (x < 0 || x >= int_width_) return;
    if (params.depth >= zbuf_[x]) return;

    const auto &[id, tex_x, tex_y0, step,
        depth, brightness,alpha] = params;
    if (alpha == 0 || step <= 0 || tex_y0 < 0) return;
    if (y0 > y1) std::swap(y0, y1); // guaranteeing y0 < y1

    int clipping_offset = 0; // if we need to offset tex_
    if (y0 < 0) {
        clipping_offset = -y0 * step, y0 = 0;
    }
    y1 = min(y1, int_height_ - 1); // top bound clip
    if (y0 > y1) return;

    Uint8 fog_factor = calc_fog_factor(depth);

    const Texture &tex = storage_.get(id);
    if (tex_x < 0 || tex_x >= tex.width) return;

    const Uint32 *tex_col = tex.column(tex_x);
    Uint32 *col = &buffer_[x * height_];

    const Uint8 *lut = core::DIM[brightness].data();
    //step is fixed precision number in 16.16 format
    for (int y = y0, tex_y = tex_y0 + clipping_offset; y <= y1; ++y, tex_y += step) {
        int tex_y_clamped = std::clamp(tex_y >> 16, 0, tex.height - 1); // converting 16.16 back to integer

        Uint32 texel = tex_col[tex_y_clamped];
        if ((texel & 0xFF000000) == 0) continue;

        Uint32 lit_texel = apply_brightness(lut, texel);
        Uint32 fogged_texel = fog_factor != 255 ? blend_argb(fog_color_, lit_texel, fog_factor) : fog_color_;

        if (alpha == 255) {
            col[y] = fogged_texel;
        } else { col[y] = blend_argb(fogged_texel, col[y], alpha); }
    }
}

void FrameBuffer::hline(const int y, int x0, int x1, const Color color) {
    if (y < 0 || y >= int_height_) return;
    if (x0 > x1) std::swap(x0, x1);
    x0 = max(x0, 0);
    x1 = min(x1, int_width_ - 1);
    if (x0 > x1) return;

    const uint32_t color_as_uint = color.toUint32_t();

    const buf_size_t h = height_;
    const auto x_end = static_cast<buf_size_t>(x1);
    for (buf_size_t x = x0; x <= x_end; ++x) {
        buffer_[x * h + y] = color_as_uint;
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

    const uint32_t color_as_uint = color.toUint32_t();

    for (int x = x0; x <= x1; ++x) {
        // TODO Add multithreading
        uint32_t *colPtr = &buffer_[x * height_];
        std::fill_n(colPtr + y0, y1 - y0 + 1, color_as_uint);
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
