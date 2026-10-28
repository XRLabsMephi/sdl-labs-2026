#pragma once

#include <cstdint>

namespace wolf {

// Packed native-endian integer 0xRRGGBBAA, matching SDL_PIXELFORMAT_RGBA8888.
// The same format can be reused by textures and sprites in lab 2.
using Color = std::uint32_t;

[[nodiscard]] constexpr Color makeColor(
        std::uint8_t red, std::uint8_t green, std::uint8_t blue,
        std::uint8_t alpha = 255) noexcept {
    return (static_cast<Color>(red) << 24) |
           (static_cast<Color>(green) << 16) |
           (static_cast<Color>(blue) << 8) |
           static_cast<Color>(alpha);
}

} // namespace wolf
