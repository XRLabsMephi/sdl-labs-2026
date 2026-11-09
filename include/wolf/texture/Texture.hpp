#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace texture {
    // Same packed RGBA format as ColorRGBA::pack(); no SDL objects in rendering code.
    struct Texture {
        int width = 0;
        int height = 0;
        std::vector<std::uint32_t> pixels;

        [[nodiscard]] bool valid() const noexcept {
            return width > 0 && height > 0 &&
                pixels.size() == static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
        }

        // Nearest-neighbour sampling with clamped edges. Invalid textures are transparent.
        [[nodiscard]] std::uint32_t sample(int x, int y) const noexcept {
            if (!valid()) return 0;
            x = std::clamp(x, 0, width - 1);
            y = std::clamp(y, 0, height - 1);
            return pixels[static_cast<std::size_t>(y) * width + x];
        }
    };
}
