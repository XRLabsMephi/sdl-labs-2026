#include "wolf/texture/Texture.hpp"

#include <cstddef>

namespace wolf {

Color Texture::sample(int x, int y) const {
    if (x < 0 || y < 0 || x >= width || y >= height) {
        return {0, 0, 0, 0};
    }
    return pixels[static_cast<std::size_t>(y) * width + x];
}

}
