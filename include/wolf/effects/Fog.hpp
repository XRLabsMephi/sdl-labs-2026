#pragma once

#include "wolf/core/Color.hpp"
#include <algorithm>
#include <cmath>

struct Fog {
    double start = 3.0;
    double end = 10.0;
    Color color = makeColor(70, 100, 140);
    Color apply(Color pixel, double depth) const {
        if (!std::isfinite(start) || !std::isfinite(end) || end <= start || std::isnan(depth)) return pixel;
        double factor = std::clamp((depth - start) / (end - start), 0.0, 1.0);
        int red = static_cast<int>(std::lround(((pixel >> 16) & 255) * (1.0 - factor) + ((color >> 16) & 255) * factor));
        int green = static_cast<int>(std::lround(((pixel >> 8) & 255) * (1.0 - factor) + ((color >> 8) & 255) * factor));
        int blue = static_cast<int>(std::lround((pixel & 255) * (1.0 - factor) + (color & 255) * factor));
        int alpha = (pixel >> 24) & 255;
        return makeColor(red, green, blue, alpha);
    }
};