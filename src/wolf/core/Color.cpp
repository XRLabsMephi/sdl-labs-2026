#include "wolf/core/Color.hpp"

Color makeColor(
    std::uint8_t red,
    std::uint8_t green,
    std::uint8_t blue,
    std::uint8_t alpha
) {
    return (static_cast<Color>(alpha) << 24)
           | (static_cast<Color>(red) << 16)
           | (static_cast<Color>(green) << 8)
           | static_cast<Color>(blue);
}
