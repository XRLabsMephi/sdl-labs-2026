#pragma once
#include <cstdint>

namespace wolf::core {

    struct Color {
        std::uint8_t r = 0;
        std::uint8_t g = 0;
        std::uint8_t b = 0;
        std::uint8_t a = 255;

        std::uint32_t Shift() const {
            return (static_cast<std::uint32_t>(a) << 24) |
                   (static_cast<std::uint32_t>(r) << 16) |
                   (static_cast<std::uint32_t>(g) << 8)  | static_cast<std::uint32_t>(b);
        }
    };
}