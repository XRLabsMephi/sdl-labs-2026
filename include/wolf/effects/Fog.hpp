#pragma once
#include "wolf/core/Color.hpp"
#include <algorithm>

namespace wolf::effects {

    class Fog {
    public:
        double fogStart = 5.0;
        double fogEnd = 13.0;
        std::uint8_t fogR = 30;
        std::uint8_t fogG = 30;
        std::uint8_t fogB = 30;

        [[nodiscard]] std::uint32_t apply(std::uint32_t color, double depth) const noexcept {
            if (depth <= fogStart) {
                return color;
            }
            if (depth >= fogEnd) {
                return 0xFF000000 |(static_cast<std::uint32_t>(fogR) << 16) | (static_cast<std::uint32_t>(fogG) << 8)
                | static_cast<std::uint32_t>(fogB);
            }
            const double coef = (depth - fogStart) / (fogEnd - fogStart);
            std::uint32_t a = (color >> 24) & 0xFF, r = (color >> 16) & 0xFF,
            g = (color >> 8) & 0xFF, b = color & 0xFF;
            // линейная интерполяция
            r = static_cast<std::uint32_t>(r * (1.0 - coef) + fogR * coef);
            g = static_cast<std::uint32_t>(g * (1.0 - coef) + fogG * coef);
            b = static_cast<std::uint32_t>(b * (1.0 - coef) + fogB * coef);
            return (a << 24) | (r << 16) | (g << 8) | b;
        }
    };
}