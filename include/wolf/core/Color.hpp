#pragma once

#include <cstdint>

namespace wolf {

constexpr std::uint32_t rgb(std::uint8_t r, std::uint8_t g, std::uint8_t b) {
    return 0xff000000u | (std::uint32_t{r} << 16) | (std::uint32_t{g} << 8) | b;
}

constexpr std::uint32_t darken(std::uint32_t color) {
    return (color & 0xff000000u) | ((color & 0x00fefefeu) >> 1);
}

constexpr std::uint32_t blend(std::uint32_t source, std::uint32_t background) {
    const auto alpha = source >> 24;
    const auto channel = [=](int shift) {
        return static_cast<std::uint8_t>((((source >> shift) & 255u) * alpha
            + ((background >> shift) & 255u) * (255u - alpha) + 127u) / 255u);
    };
    return rgb(channel(16), channel(8), channel(0));
}

}
