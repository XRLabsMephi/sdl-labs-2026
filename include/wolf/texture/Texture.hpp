#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

namespace wolf::texture {
    struct Texture {
        int width = 0;
        int height = 0;
        std::vector<std::uint32_t> pixels;
        [[nodiscard]] std::uint32_t sample(int x, int y) const {
            if (width <= 0 || height <= 0 || pixels.empty()) {
                return 0;
            }
            const int clampX = std::clamp(x, 0, width - 1);
            const int clampY = std::clamp(y, 0, height - 1);
            return pixels[static_cast<std::size_t>(clampY) * width + clampX];
        }

        [[nodiscard]] bool valid() const noexcept {
            return width > 0 && height > 0 && pixels.size() == static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
        }
    };
}