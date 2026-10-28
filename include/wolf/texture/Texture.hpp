#pragma once

#include "wolf/core/Color.hpp"

#include <cstddef>
#include <stdexcept>
#include <vector>

namespace wolf {

struct Texture {
    int width = 0;
    int height = 0;
    std::vector<Color> pixels;

    [[nodiscard]] Color sample(int x, int y) const {
        if (x < 0 || y < 0 || x >= width || y >= height) {
            throw std::out_of_range("Texture coordinates out of bounds");
        }
        return pixels.at(static_cast<std::size_t>(y) * static_cast<std::size_t>(width) +
                         static_cast<std::size_t>(x));
    }
};

} // namespace wolf
