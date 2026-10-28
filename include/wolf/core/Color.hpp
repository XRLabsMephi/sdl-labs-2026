#pragma once

#include <cstdint>

namespace wolf {

    using Color = std::uint32_t;

    [[nodiscard]] constexpr Color makeColor(
            std::uint8_t red,
            std::uint8_t green,
            std::uint8_t blue,
            std::uint8_t alpha = 255) noexcept {
        return (static_cast<Color>(red) << 24) |
               (static_cast<Color>(green) << 16) |
               (static_cast<Color>(blue) << 8) |
                static_cast<Color>(alpha);
    }

}
