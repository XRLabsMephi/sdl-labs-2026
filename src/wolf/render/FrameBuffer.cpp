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
      , max_x_(0)
      , max_y_(0) {
    if (width <= 0) { throw std::invalid_argument("width must be > 0"); }
    if (height <= 0) { throw std::invalid_argument("height must be > 0"); }

    width_ = static_cast<buf_size_t>(width);
    height_ = static_cast<buf_size_t>(height);

    if (width_ > std::numeric_limits<buf_size_t>::max() / height_) {
        throw std::length_error("Buffer dimensions are too large");
    }

    max_x_ = width - 1;
    max_y_ = height - 1;

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

void FrameBuffer::set(int x, int y, const Color color) {
    if (x < 0 || y < 0 || x > max_x_ || y > max_y_) return;

    buffer_[x * height_ + y] = static_cast<Uint32>(color);
}

void FrameBuffer::vline(int x, int y0, int y1, const Color color) {
    if (x < 0 || x > max_x_) return;
    if (y0 > y1) std::swap(y0, y1);

    y0 = max(y0, 0);
    y1 = min(y1, max_y_);
    if (y0 > y1) return;

    uint32_t *col = &buffer_[x * height_];
    std::fill_n(col + y0, y1 - y0 + 1, static_cast<Uint32>(color));
}

[[gnu::noinline]] void FrameBuffer::buildFogLut(Uint8 brightness, Uint8 fog_factor) {
    const Uint8 *lut = core::DIM[brightness].data();
    const Uint32 fog_rgb = static_cast<Uint32>(fog_.rgb_color) & 0x00FFFFFFu;
    const Uint32 fog_r = (fog_rgb >> 16) & 0xFFu;
    const Uint32 fog_g = (fog_rgb >> 8) & 0xFFu;
    const Uint32 fog_b = fog_rgb & 0xFFu;
    const Uint32 ft = fog_factor;
    const Uint32 finv = 255u - ft;
    for (int v = 0; v < 256; ++v) {
        const Uint32 d = lut[v];
        lut_r_[v] = static_cast<Uint8>((finv * d + ft * fog_r + 127u) / 255u);
        lut_g_[v] = static_cast<Uint8>((finv * d + ft * fog_g + 127u) / 255u);
        lut_b_[v] = static_cast<Uint8>((finv * d + ft * fog_b + 127u) / 255u);
    }
    lut_fog_q_ = fog_factor >> 3;
    lut_brightness_ = brightness;
}

template<bool IsSprite>
void FrameBuffer::vline(int x, int y0, int y1, const DrawCallParams &params) {
    if (x < 0 || x > max_x_) return;

    const auto &[tex_col, tex_y0, step, depth, brightness] = params;
    if (!tex_col || step < 0 || tex_y0 < 0) return;
    if (y0 > y1) {
       std::swap(y0, y1);
    }

    int clipping_offset = 0;
    if (y0 < 0) { clipping_offset = -y0 * step, y0 = 0; }
    y1 = min(y1, max_y_);
    if (y0 > y1) return;

    const Uint8 fog_factor = fog_.factor(static_cast<float>(depth));
    const auto fog_rgb = static_cast<Uint32>(fog_.rgb_color);

    Uint32 *__restrict cp = &buffer_[x * height_];
    const Uint32 *__restrict tc = tex_col;

    // LUT path for fog_factor in [1, 254]
    if (fog_factor > 0 && fog_factor < 255) {
        const Uint8 fog_q = fog_factor >> 3; // fog key (quantized to 32/step)
        if (lut_fog_q_ != fog_q || lut_brightness_ != brightness) { //if fog or brightness changed too much - rebuild
            buildFogLut(brightness, fog_q << 3);
        }

        for (int y = y0, tex_y = tex_y0 + clipping_offset; y <= y1; ++y, tex_y += step) {
            const Uint32 texel = tc[tex_y >> 16]; //unpacking 16.16 value and getting the texel
            const Uint32 texel_alpha = texel & 0xFF000000u;
            if constexpr (IsSprite) { if (!texel_alpha) continue; }

            // applying fog as raw Uint32 lerp
            const Uint32 fogged = texel_alpha
                            | (static_cast<Uint32>(lut_r_[(texel >> 16) & 0xFFu]) << 16)
                            | (static_cast<Uint32>(lut_g_[(texel >> 8) & 0xFFu]) << 8)
                            | static_cast<Uint32>(lut_b_[texel & 0xFFu]);

            //walls are opaque, so no need to blend
            if constexpr (!IsSprite) {
                cp[y] = fogged;
            } else {
                //if sprite pixel is opaque no need to blend
                if (texel_alpha == 0xFF000000u) {
                    cp[y] = fogged;
                    continue;
                }
                // raw Uint32 "over" type of blend
                const Uint32 dst = cp[y];
                const Uint32 sa = texel_alpha >> 24;
                const Uint8 *da = core::DIM[sa].data();
                const Uint8 *dia = core::DIM[255 - sa].data();
                cp[y] = 0xFF000000u |
                        ((da[(fogged >> 16) & 0xFF] + dia[(dst >> 16) & 0xFF]) << 16) |
                        ((da[(fogged >> 8) & 0xFF] + dia[(dst >> 8) & 0xFF]) << 8) |
                        (da[fogged & 0xFF] + dia[dst & 0xFF]);
            }
        }
        return;
    }

    //non-combined LUT
    const Uint8 *lut = core::DIM[brightness].data();

    //for walls no need to blend
    if constexpr (!IsSprite) {
        if (fog_factor == 255) {
            const Uint32 solid = 0xFF000000u | fog_rgb;
            std::fill_n(cp + y0, y1 - y0 + 1, solid);
            return;
        }

        // fog_factor == 0
        for (int y = y0, tex_y = tex_y0 + clipping_offset; y <= y1; ++y, tex_y += step) {
            const Uint32 texel = tc[tex_y >> 16];
            //applying brightness
            const Uint32 r = lut[(texel >> 16) & 0xFFu];
            const Uint32 g = lut[(texel >> 8) & 0xFFu];
            const Uint32 b = lut[texel & 0xFFu];
            cp[y] = 0xFF000000u | (r << 16) | (g << 8) | b;
        }
        return;
    }

    for (int y = y0, tex_y = tex_y0 + clipping_offset; y <= y1; ++y, tex_y += step) {
        const Uint32 texel = tc[tex_y >> 16];
        const Uint32 texel_alpha = texel & 0xFF000000u;
        if (!texel_alpha) continue;

        //applying brightness
        Uint32 lit = 0xFF000000u |
                     (static_cast<Uint32>(lut[(texel >> 16) & 0xFFu]) << 16) |
                     (static_cast<Uint32>(lut[(texel >> 8) & 0xFFu]) << 8) |
                     static_cast<Uint32>(lut[texel & 0xFFu]);

        if (fog_factor == 255) lit = 0xFF000000u | fog_rgb;
        else if (fog_factor != 0) {
            // applyFog manually
            const Uint32 t = fog_factor;
            const Uint32 inv = 255u - t;
            lit = 0xFF000000u |
                  (((inv * ((lit >> 16) & 0xFFu) + t * ((fog_rgb >> 16) & 0xFFu) + 127u) / 255u) << 16) |
                  (((inv * ((lit >> 8) & 0xFFu) + t * ((fog_rgb >> 8) & 0xFFu) + 127u) / 255u) << 8) |
                  ((inv * (lit & 0xFFu) + t * (fog_rgb & 0xFFu) + 127u) / 255u);
        }

        if (texel_alpha == 0xFF000000u) {
            cp[y] = lit;
            continue;
        }

        const Uint32 dst = cp[y];
        const Uint32 sa = texel_alpha >> 24;
        const Uint8 *da = core::DIM[sa].data();
        const Uint8 *dia = core::DIM[255 - sa].data();
        //manual Uint32 blend
        cp[y] = 0xFF000000u |
                ((da[(lit >> 16) & 0xFF] + dia[(dst >> 16) & 0xFF]) << 16) |
                ((da[(lit >> 8) & 0xFF] + dia[(dst >> 8) & 0xFF]) << 8) |
                (da[lit & 0xFF] + dia[dst & 0xFF]);
    }
}

template void FrameBuffer::vline<false>(int, int, int, const DrawCallParams &);

template void FrameBuffer::vline<true>(int, int, int, const DrawCallParams &);

void FrameBuffer::vline_sprite(int x, int y0, int y1, const DrawCallParams &params) {
    vline<true>(x, y0, y1, params);
}

void FrameBuffer::hline(const int y, int x0, int x1, const Color color) {
    if (y < 0 || y > max_y_) return;
    if (x0 > x1) std::swap(x0, x1);
    x0 = max(x0, 0);
    x1 = min(x1, max_x_);
    if (x0 > x1) return;

    const buf_size_t h = height_;
    const auto x_end = static_cast<buf_size_t>(x1);
    for (buf_size_t x = x0; x <= x_end; ++x) {
        buffer_[x * h + y] = static_cast<Uint32>(color);
    }
}

void FrameBuffer::hline(const int y, int x0, int x1, const DrawCallParams &params) {
    if (y < 0 || y > max_y_) return;
    if (x0 > x1) std::swap(x0, x1);
    x0 = max(x0, 0);
    x1 = min(x1, max_x_);
    if (x0 > x1) return;

    const auto &[tex_col, tex_y0, step, depth, brightness] = params;
    if (!tex_col || step < 0 || tex_y0 < 0) return;
    //TODO
}

void FrameBuffer::rectangle(int x0, int y0, int x1, int y1, const Color color) {
    if (y0 > y1) std::swap(y0, y1);
    if (x0 > x1) std::swap(x0, x1);

    y0 = max(y0, 0);
    x0 = max(x0, 0);
    x1 = min(x1, max_x_);
    y1 = min(y1, max_y_);

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
    x1 = min(x1, max_x_ - 1);
    y1 = min(y1, max_y_ - 1);

    if (x0 > x1 || y0 > y1) return;

    const auto &[tex_col, tex_y0, step, depth, brightness] = params;
    if (!tex_col || step < 0 || tex_y0 < 0) return;
    //TODO
}
