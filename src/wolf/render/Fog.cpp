#include "wolf/render/Fog.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace wolf {

Color Fog::apply(Color source, double depth) const {
    if (!std::isfinite(start) || !std::isfinite(end) || start < 0 || start >= end) {
        throw std::invalid_argument("Fog requires finite distances with 0 <= start < end");
    }
    const double factor = std::clamp((depth - start) / (end - start), 0.0, 1.0);
    auto blend = [factor](std::uint8_t from, std::uint8_t to) {
        return static_cast<std::uint8_t>(std::lround(std::lerp(from, to, factor)));
    };
    return {blend(source.r, color.r), blend(source.g, color.g),
            blend(source.b, color.b), source.a};
}

}
