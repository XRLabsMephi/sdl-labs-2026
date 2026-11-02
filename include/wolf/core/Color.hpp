#pragma once

#include <cstdint>

using Color = std::uint32_t;

Color makeColor(
    std::uint8_t red,
    std::uint8_t green,
    std::uint8_t blue,
    std::uint8_t alpha = 255
);