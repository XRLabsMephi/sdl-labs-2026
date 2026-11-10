#include <render/FrameBuffer.h>
#include <stdexcept>
#include <ranges>
#include <algorithm>
#include <limits>

#include "debug/Logger.h"
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

void FrameBuffer::set(int x, int y, const Color color) {
    if (x < 0 || y < 0 || x >= int_width_ || y >= int_height_) return;

    buffer_[x * height_ + y] = static_cast<Uint32>(color);
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

void FrameBuffer::vline(int x, int y0, int y1, const DrawCallParams &params) {
    if (x < 0 || x >= int_width_) return;

    const auto &[texture, tex_y0, step, depth, brightness] = params;
    if (!texture || step < 0 || tex_y0 < 0) return;
    if (y0 > y1) std::swap(y0, y1);

    int clipping_offset = 0;
    if (y0 < 0) { clipping_offset = -y0 * step, y0 = 0; }
    y1 = min(y1, int_height_ - 1);
    if (y0 > y1) return;

    const Uint8 fog_factor = fog_.factor(depth);
    const auto fog_color = static_cast<Uint32>(fog_.color);
    const Uint32 *__restrict tex_col = texture;
    Uint32 *__restrict col = &buffer_[x * height_];

    if (fog_factor == 255) {
        std::fill(col + y0, col + y1 + 1, 0xFF000000u | fog_color);
        return;
    }

    const Uint8 *__restrict lut = core::DIM[brightness].data();

    const auto render_wall_texel = [&](const int texel_idx, const int y_start, const int run) {
        const Uint32 texel = tex_col[texel_idx];
        Uint32 lit = static_cast<Uint32>(Color::fromBits(texel).dimmed(lut));

        if (fog_factor != 0) {
            lit = fog_.applyFog(lit, fog_factor);
        }
        std::fill_n(col + y_start, run, lit);
    };

    if (step == 0) {
        render_wall_texel((tex_y0 + clipping_offset) >> 16, y0, y1 - y0 + 1);
        return;
    }

    if (step >= 65536) {
        int tex_y = tex_y0 + clipping_offset;
        for (int y = y0; y <= y1; ++y, tex_y += step) {
            render_wall_texel(tex_y >> 16, y, 1);
        }
        return;
    }

    // Magnified (стена близко) — считаем ран и заливаем пачкой
    int y = y0;
    int texel_idx = (tex_y0 + clipping_offset) >> 16;
    int overshoot = (tex_y0 + clipping_offset) & 0xFFFF;
    int run = (65536 - overshoot + step - 1) / step;
    if (y + run > y1 + 1) run = y1 + 1 - y;

    render_wall_texel(texel_idx, y, run);

    y += run;
    ++texel_idx;
    if (y > y1) return;
    overshoot += run * step - 65536;

    const int base = 65536 / step;
    const int rem = 65536 - base * step;

    while (y <= y1) {
        int r;
        if (overshoot >= rem) { r = base; overshoot -= rem; }
        else { r = base + 1; overshoot += step - rem; }
        if (y + r > y1 + 1) r = y1 + 1 - y;

        render_wall_texel(texel_idx, y, r);

        y += r;
        ++texel_idx;
    }
}

void FrameBuffer::vline_sprite(int x, int y0, int y1, const DrawCallParams &params) {
    if (x < 0 || x >= int_width_) return;

    const auto &[texture, tex_y0, step, depth, brightness] = params;
    if (!texture || step < 0 || tex_y0 < 0) return;
    if (y0 > y1) std::swap(y0, y1);

    int clipping_offset = 0;
    if (y0 < 0) { clipping_offset = -y0 * step; y0 = 0; }
    y1 = min(y1, int_height_ - 1);
    if (y0 > y1) return;

    const Uint8 fog_factor = fog_.factor(depth);
    const auto fog_color = static_cast<Uint32>(fog_.color);
    const Uint32 *__restrict tex_col = texture;
    Uint32 *__restrict col = &buffer_[x * height_];
    const Uint8 *lut = core::DIM[brightness].data();
    const int tex_y_start = tex_y0 + clipping_offset;
    const Uint32 fill_const = 0xFF000000u | fog_color;

    const auto render_run = [&](const int idx, const int y_start, const int run) {
        const Uint32 texel = tex_col[idx];
        const Uint32 a = texel & 0xFF000000u;
        if (!a) return;

        Uint32 lit;
        if (fog_factor == 255) { lit = fill_const; }
        else {
            lit = static_cast<Uint32>(Color::fromBits(texel).dimmed(lut));
            if (fog_factor) lit = fog_.applyFog(lit, fog_factor);
        }

        if (a == 0xFF000000u) {
            std::fill_n(col + y_start, run, lit);
        } else {
            const auto c = Color::fromBits(lit);
            for (int k = y_start; k < y_start + run; ++k) {
                col[k] = static_cast<Uint32>(c.over(Color::fromBits(col[k])));
            }
        }
    };

    if (step == 0) {
        render_run(tex_y_start >> 16, y0, y1 - y0 + 1);
        return;
    }

    if (step >= 65536) {
        for (int y = y0, tex_y = tex_y_start; y <= y1; ++y, tex_y += step) {
            render_run(tex_y >> 16, y, 1);
        }
        return;
    }

    int y = y0;
    int texel_idx = tex_y_start >> 16;
    int overshoot = tex_y_start & 0xFFFF;
    int run = (65536 - overshoot + step - 1) / step;
    if (y + run > y1 + 1) run = y1 + 1 - y;
    render_run(texel_idx, y, run);
    y += run;
    ++texel_idx;
    if (y > y1) return;
    overshoot += run * step - 65536;

    const int base = 65536 / step;
    const int rem = 65536 - base * step;
    while (y <= y1) {
        int r;
        if (overshoot >= rem) { r = base; overshoot -= rem; }
        else { r = base + 1; overshoot += step - rem; }
        if (y + r > y1 + 1) r = y1 + 1 - y;
        render_run(texel_idx, y, r);
        y += r;
        ++texel_idx;
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
    const auto &[tex_col, tex_y0, step, depth, brightness] = params;
    if (!tex_col || step < 0 || tex_y0 < 0) {
    }
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
    const auto &[tex_col, tex_y0, step, depth, brightness] = params;
    if (!tex_col || step < 0 || tex_y0 < 0) {
    }
    //TODO
}
