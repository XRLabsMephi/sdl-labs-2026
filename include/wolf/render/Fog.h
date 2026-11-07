#pragma once
#include <limits>

#include "core/Color.h"

struct Fog {
    float min_depth = std::numeric_limits<float>::infinity(); /**< < => no fog*/
    float max_depth = std::numeric_limits<float>::infinity(); /**< >= => fog_factor = 1*/
    core::Color color;

    [[nodiscard]] Uint8 factor(float depth) const {
        if (depth < min_depth) return 0;
        if (depth >= max_depth) { return 255; }
        return static_cast<Uint8>(255.0 * (depth - min_depth) / (max_depth - min_depth));
    }

    [[nodiscard]] Uint32 applyFog(Uint32 col, Uint8 fog_factor) const {
        if (fog_factor == 0) return col;
        if (fog_factor == 255) return (col & 0xFF000000u) | (static_cast<Uint32>(color) & 0x00FFFFFFu);
        const auto *da = core::DIM[fog_factor].data();
        const auto *dia = core::DIM[255 - fog_factor].data();
        const Uint32 r = da[col >> 16 & 0xFF] + dia[static_cast<Uint32>(color) >> 16 & 0xFF];
        const Uint32 g = da[col >> 8 & 0xFF] + dia[static_cast<Uint32>(color) >> 8 & 0xFF];
        const Uint32 b = da[col & 0xFF] + dia[static_cast<Uint32>(color) & 0xFF];
        return (col & 0xFF000000u) | r << 16 | g << 8 | b;
    }
};
