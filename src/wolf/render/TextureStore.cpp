#include "wolf/render/TextureStore.hpp"

#include <SDL3/SDL.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

namespace wolf
{

void TextureStore::loadBmp(int id, const char* path, bool colorKey)
{
    using Surface = std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)>;
    Surface image(SDL_LoadBMP(path), SDL_DestroySurface);
    if (!image)
    {
        throw std::runtime_error(std::string(path) + ": " + SDL_GetError());
    }

    Texture texture{image->w, image->h, {}};
    texture.pixels.reserve(static_cast<std::size_t>(image->w) * image->h);
    for (int y = 0; y < image->h; ++y)
    {
        for (int x = 0; x < image->w; ++x)
        {
            Uint8 red, green, blue;
            if (!SDL_ReadSurfacePixel(image.get(), x, y, &red, &green, &blue, nullptr))
            {
                throw std::runtime_error(SDL_GetError());
            }
            const auto rgb = (std::uint32_t{red} << 16U) | (std::uint32_t{green} << 8U) | blue;
            texture.pixels.push_back(colorKey && rgb == 0xFF00FFU ? 0 : 0xFF000000U | rgb);
        }
    }
    textures_.insert_or_assign(id, std::move(texture));
}

} // namespace wolf
