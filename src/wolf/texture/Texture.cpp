#include "wolf/texture/Texture.hpp"

#include <cstddef>
#include <stdexcept>

namespace wolf
{

bool Texture::valid() const noexcept
{
    return width > 0 && height > 0
        && pixels.size() == static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
}

Texture::Pixel Texture::sample(int x, int y) const
{
    if (!valid() || x < 0 || x >= width || y < 0 || y >= height)
    {
        throw std::out_of_range("Texture sample coordinates are outside the image");
    }
    return pixels[static_cast<std::size_t>(y) * static_cast<std::size_t>(width)
        + static_cast<std::size_t>(x)];
}

} // namespace wolf
