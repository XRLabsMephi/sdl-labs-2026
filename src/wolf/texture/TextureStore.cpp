#include "wolf/texture/TextureStore.hpp"

#include <SDL3/SDL.h>

#include <cstring>
#include <memory>

namespace wolf {

void TextureStore::load(int id, const std::filesystem::path& path,
                        std::optional<std::uint32_t> colorKey) {
    using Surface = std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)>;
    const auto filename = path.u8string();
    Surface original(SDL_LoadBMP(reinterpret_cast<const char*>(filename.c_str())), SDL_DestroySurface);
    if (!original) {
        throw std::runtime_error("Не удалось загрузить текстуру: " + std::string(SDL_GetError()));
    }
    Surface surface(SDL_ConvertSurface(original.get(), SDL_PIXELFORMAT_ARGB8888), SDL_DestroySurface);
    if (!surface) {
        throw std::runtime_error(SDL_GetError());
    }
    Texture texture{surface->w, surface->h, {}};
    texture.pixels.resize(static_cast<std::size_t>(texture.width) * texture.height);
    if (!SDL_LockSurface(surface.get())) {
        throw std::runtime_error(SDL_GetError());
    }
    for (int y = 0; y < texture.height; ++y) {
        const auto* row = static_cast<const std::uint8_t*>(surface->pixels)
            + static_cast<std::size_t>(y) * surface->pitch;
        std::memcpy(texture.pixels.data() + static_cast<std::size_t>(y) * texture.width,
                    row, static_cast<std::size_t>(texture.width) * sizeof(std::uint32_t));
    }
    SDL_UnlockSurface(surface.get());
    if (colorKey) {
        for (auto& pixel : texture.pixels) {
            if ((pixel & 0x00ffffffu) == (*colorKey & 0x00ffffffu)) {
                pixel = 0;
            }
        }
    }
    set(id, std::move(texture));
}

}
