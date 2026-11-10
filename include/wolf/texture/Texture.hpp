#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace texture {
    // Same packed RGBA format as ColorRGBA::pack()
    struct Texture {
        size_t width = 0;
        size_t height = 0;
        std::vector<uint32_t> pixels;

        bool valid() const noexcept {
            return width > 0 && height > 0 && pixels.size() == width * height;
        }

        // Nearest-neighbour sampling with clamped edges. Invalid textures are transparent.
        uint32_t sample(int x, int y) const noexcept {
            if (!valid()) {
                return 0;
            }
            size_t clx = std::min((size_t) std::max(x, 0), width - 1);
            size_t cly = std::min((size_t) std::max(y, 0), height - 1);
            return pixels[cly * width + clx];
        }
    };
}
