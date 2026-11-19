#pragma once

#include <algorithm>
#include <cmath>
#include <limits>
#include "FrameBuffer.h"

namespace render {
    // Build a clipped, half-open vertical span and keep all samples inside the texture.
    inline bool projectTextureColumn(double top, double bottom, int frame_height,
                                     int texture_height, int& y0, int& y_end,
                                     DrawCallParams& params) {
        constexpr int unit = 1 << 16;
        if (!std::isfinite(top) || !std::isfinite(bottom) || top >= bottom ||
            texture_height <= 0 || texture_height > std::numeric_limits<int>::max() / unit) {
            return false;
        }
        y0 = static_cast<int>(std::ceil(std::clamp(top, 0.0, static_cast<double>(frame_height))));
        y_end = static_cast<int>(std::ceil(std::clamp(bottom, 0.0, static_cast<double>(frame_height))));
        if (y0 >= y_end) return false;

        const int max_sample = texture_height * unit - 1;
        const double height = bottom - top;
        params.tex_y_start = static_cast<int>(std::clamp(
            ((y0 - top) / height) * (texture_height * unit), 0.0, static_cast<double>(max_sample)));
        const int steps = y_end - y0 - 1;
        params.tex_step = steps == 0 ? 0 : static_cast<int>(std::min(
            (texture_height * unit) / height,
            static_cast<double>((max_sample - params.tex_y_start) / steps)));
        return true;
    }
}
