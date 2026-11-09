#pragma once

#include <algorithm>
#include <cstdint>

namespace wolf
{
using Color = std::uint32_t;

constexpr Color makeColor(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a = 255)
{
    return (static_cast<Color>(a) << 24) | (static_cast<Color>(r) << 16) | (static_cast<Color>(g) << 8) | static_cast<Color>(b);
}

inline Color shadeColor(Color color, double factor)
{
    factor = std::clamp(factor, 0.0, 1.0);

    const auto r = static_cast<std::uint8_t>(((color >> 16) & 0xFF) * factor);
    const auto g = static_cast<std::uint8_t>(((color >> 8) & 0xFF) * factor);
    const auto b = static_cast<std::uint8_t>((color & 0xFF) * factor);

    return makeColor(r, g, b);
}
}
