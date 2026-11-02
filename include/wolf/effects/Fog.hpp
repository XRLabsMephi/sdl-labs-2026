#pragma once

#include "wolf/core/Color.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace wolf {

    namespace {

        [[nodiscard]] std::uint8_t mix(unsigned shift, double amount, Color a, Color b) {
            const auto from = static_cast<double>((a >> shift) & 0xffu);
            const auto to = static_cast<double>((b >> shift) & 0xffu);
            return static_cast<std::uint8_t>(std::lround(from + (to - from) * amount));
        }

    }

    struct FogSettings {
        double start = 4.0;
        double end = 12.0;
        Color color = makeColor(105, 112, 124);
    };

    [[nodiscard]] inline Color applyFog(Color source, double depth, const FogSettings& fog = {}) noexcept {
        if (std::isnan(depth) ||
            !std::isfinite(fog.start) ||
            !std::isfinite(fog.end) ||
            fog.end <= fog.start ||
            depth <= fog.start) {
            return source;
        }
        const double amount = depth >= fog.end ?
                              1.0 :
                              std::clamp((depth - fog.start) / (fog.end - fog.start), 0.0, 1.0);
        return makeColor(mix(24, amount, source, fog.color),
                         mix(16, amount, source, fog.color),
                         mix(8, amount, source, fog.color),
                         static_cast<std::uint8_t>(source & 0xffu));
    }

}
