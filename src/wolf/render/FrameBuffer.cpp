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

void FrameBuffer::vline(int x, int y0, int y1, const DrawCallParams &params) {
    if (x < 0 || x >= int_width_) return;

    const auto &[id,
        tex_x,
        tex_y0,
        step,
        depth,
        brightness,
        alpha] = params;
    if (zbuf_[x] < depth) return;
    if (step <= 0) return;
    if (tex_y0 < 0) return;

    if (y0 > y1) std::swap(y0, y1);

    int tex_y_offset = 0;
    if (y0 < 0) {
        tex_y_offset = -y0 * step;
        y0 = 0;
    }
    y1 = min(y1, int_height_ - 1);
    if (y0 > y1) return;

    const Texture &tex = storage_.get(id);
    if (tex_x < 0 || tex_x >= tex.width || tex.height == 0) return;

    const Uint32 *tex_col = tex.column(tex_x);
    Uint32 *col = &buffer_[x * height_];

    const Uint8 *lut = core::DIM[brightness].data();

    if (alpha == 255) {
        //opaque draw
        //step is fixed precision number in 16.16 format
        for (int y = y0, tex_y = tex_y0 + tex_y_offset; y <= y1; ++y, tex_y += step) {
            int clamped_tex_y = tex_y >> 16;
            clamped_tex_y = clamped_tex_y >= tex.height ? tex.height - 1 : clamped_tex_y;
            //need to get rid of fraction part in tex_y
            col[y] = apply_brightness(lut, tex_col[clamped_tex_y]);
        }
        zbuf_[x] = depth;
    } else {
        //blend mode draw
        for (int y = y0, tex_y = tex_y0 + tex_y_offset; y <= y1; ++y, tex_y += step) {
            int clamped_tex_y = tex_y >> 16;
            clamped_tex_y = clamped_tex_y >= tex.height ? tex.height - 1 : clamped_tex_y;
            col[y] = blend_argb(apply_brightness(lut, tex_col[clamped_tex_y]), col[y], alpha);
        }
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
