#pragma once

#include <cstdint>

namespace wolf {

struct Color {
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
    std::uint8_t a = 255;

    bool operator==(const Color&) const = default;
};

static_assert(sizeof(Color) == 4);

}
