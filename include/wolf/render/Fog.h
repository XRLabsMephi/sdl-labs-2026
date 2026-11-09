#pragma once
#include <limits>

#include "core/Color.h"

struct Fog {
    float min_depth = std::numeric_limits<float>::infinity(); /**< < => no fog*/
    float max_depth = std::numeric_limits<float>::infinity(); /**< >= => fog_factor = 1*/
    core::Color rgb_color; /**< color of the fog. A component always 0x00*/

    Fog() = default;

    Fog(const float min_depth, const float max_depth, const core::Color color)
    : min_depth(min_depth)
    , max_depth(max_depth)
    , rgb_color(core::Color::fromBits(static_cast<Uint32>(color) & 0x00FFFFFFu)){
    }

    [[nodiscard]] Uint8 factor(float depth) const {
        if (depth < min_depth) return 0;
        if (depth >= max_depth) { return 255; }
        return static_cast<Uint8>(255.0 * (depth - min_depth) / (max_depth - min_depth));
    }

    [[nodiscard]] Uint32 applyFog(Uint32 col, Uint8 fog_factor) const {
        if (fog_factor == 0) return col;
        if (fog_factor == 255) return (col & 0xFF000000u) | static_cast<Uint32>(rgb_color);
        const auto mixed = core::Color::fromBits(col).lerp_rgb(rgb_color, fog_factor);
        return static_cast<Uint32>(mixed);
    }
};
