#pragma once

#include "wolf/core/Color.hpp"

#include <algorithm>
#include <cmath>

namespace wolf {

struct Fog {
    double start = 4.0;
    double end = 11.0;
    std::uint32_t color = rgb(85, 98, 115);

    std::uint32_t apply(std::uint32_t source, double depth) const {
        if (depth <= start) return source;
        const double factor = end > start ? std::clamp((depth - start) / (end - start), 0.0, 1.0) : 1.0;
        const auto channel = [=, this](int shift) {
            const auto a = (source >> shift) & 255u;
            const auto b = (color >> shift) & 255u;
            return static_cast<std::uint32_t>(std::lround(a * (1.0 - factor) + b * factor));
        };
        return (source & 0xff000000u) | (channel(16) << 16) | (channel(8) << 8) | channel(0);
    }
};

}
