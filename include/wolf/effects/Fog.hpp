#pragma once

#include "wolf/core/Color.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace wolf {

struct FogSettings {
    double start = 4.0;
    double end = 12.0;
    Color color = makeColor(105, 112, 124);
};

// Blend RGB toward fogColor using camera-space depth; preserve source alpha.
[[nodiscard]] inline Color applyFog(Color source, double depth,
                                    const FogSettings& fog = {}) noexcept {
    if (std::isnan(depth) || !std::isfinite(fog.start) ||
        !std::isfinite(fog.end) || fog.end <= fog.start || depth <= fog.start) {
        return source;
    }
    const double amount = depth >= fog.end ? 1.0 :
        std::clamp((depth - fog.start) / (fog.end - fog.start), 0.0, 1.0);
    const auto mix = [amount](unsigned shift, Color a, Color b) -> std::uint8_t {
        const double from = static_cast<double>((a >> shift) & 0xffu);
        const double to = static_cast<double>((b >> shift) & 0xffu);
        return static_cast<std::uint8_t>(std::lround(from + (to - from) * amount));
    };
    return makeColor(mix(24, source, fog.color), mix(16, source, fog.color),
                     mix(8, source, fog.color),
                     static_cast<std::uint8_t>(source & 0xffu));
}

} // namespace wolf
