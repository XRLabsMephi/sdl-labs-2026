#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace wolf
{

struct Fog
{
    double start = 3.0;
    double end = 12.0;
    std::uint32_t color = 0xFF3D4858;

    [[nodiscard]] std::uint32_t apply(std::uint32_t pixel, double depth) const
    {
        const double factor = std::clamp((depth - start) / (end - start), 0.0, 1.0);
        const auto channel = [=, this](unsigned shift)
        {
            const auto source = (pixel >> shift) & 0xFFU;
            const auto target = (color >> shift) & 0xFFU;
            return static_cast<std::uint32_t>(std::lround(std::lerp(source, target, factor))) << shift;
        };
        return (pixel & 0xFF000000U) | channel(16) | channel(8) | channel(0);
    }
};

} // namespace wolf
