#pragma once
#include <cstdint>

using Color = std::uint32_t;

// Формат: 0xAARRGGBB
inline constexpr Color rgb(std::uint8_t r, std::uint8_t g, std::uint8_t b) {
    return (0xFFu << 24) | (std::uint32_t(r) << 16)
           | (std::uint32_t(g) << 8)
           |  std::uint32_t(b);
}

inline constexpr std::uint8_t red  (Color c) { return (c >> 16) & 0xFF; }
inline constexpr std::uint8_t green(Color c) { return (c >>  8) & 0xFF; }
inline constexpr std::uint8_t blue (Color c) { return  c        & 0xFF; }
inline constexpr std::uint8_t alpha(Color c) { return (c >> 24) & 0xFF; }