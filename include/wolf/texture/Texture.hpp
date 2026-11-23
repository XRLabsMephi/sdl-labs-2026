#pragma once

#include <cstdint>
#include <vector>

namespace wolf
{

struct Texture
{
    using Pixel = std::uint32_t;

    int width = 0;
    int height = 0;
    std::vector<Pixel> pixels;

    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] Pixel sample(int x, int y) const;
};

} // namespace wolf
