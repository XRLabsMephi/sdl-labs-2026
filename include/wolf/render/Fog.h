#pragma once
#include <limits>

#include "FrameBuffer.h"
#include "core/Color.h"

struct Fog {
    float min_depth = std::numeric_limits<float>::infinity(); /**< < => no fog*/
    float max_depth = std::numeric_limits<float>::infinity(); /**< >= => fog_factor = 1*/
    core::Color color;

    Fog() = default;
    Fog(float min, float max, core::Color col)
    : min_depth(min)
    , max_depth(max)
    , color(core::Color::fromBits(static_cast<Uint32>(col) & 0x00FFFFFFu)){}

    [[nodiscard]] Uint8 factor(const double depth) const {
        if (depth < min_depth) return 0;
        if (depth >= max_depth) { return 255; }
        return static_cast<Uint8>(255.0 * (depth - min_depth) / (max_depth - min_depth));
    }

    [[nodiscard]] Uint32 applyFog(Uint32 col, Uint8 fog_factor) const {
        if (fog_factor == 0) return col;
        if (fog_factor == 255) return (col & 0xFF000000u) | (static_cast<Uint32>(color) & 0x00FFFFFFu);
        const auto mixed = core::Color::fromBits(col).lerp_rgb(color, fog_factor);
        return (col & 0xFF000000u) | (static_cast<Uint32>(mixed) & 0x00FFFFFFu);
    }
};
