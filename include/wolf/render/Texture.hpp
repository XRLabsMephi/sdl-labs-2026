#pragma once

#include <cstdint>
#include <vector>

namespace wolf
{

struct Texture
{
    int width;
    int height;
    std::vector<std::uint32_t> pixels;

    [[nodiscard]] std::uint32_t sample(int x, int y) const
    {
        return pixels[static_cast<std::size_t>(y) * width + x];
    }
};

} // namespace wolf
