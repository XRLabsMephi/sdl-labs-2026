#include "wolf/texture/TextureStore.hpp"
#include <SDL3/SDL.h>
#include <format>
#include <stdexcept>
#include <memory>

namespace wolf::texture {
    void TextureStore::load(int id, const std::string& filepath) {
        std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)> rawSurface(SDL_LoadBMP(filepath.c_str()), SDL_DestroySurface);

        if (!rawSurface) {
            throw std::runtime_error("failed to load texture from " + filepath);
        }
        std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)> surface(SDL_ConvertSurface(rawSurface.get(), SDL_PIXELFORMAT_RGBA32), SDL_DestroySurface);
        if (!surface) {
            throw std::runtime_error("failed to load texture from " + filepath);
        }
        //surface и питч создает сдл
        Texture tex;
        tex.width = surface->w;
        tex.height = surface->h;
        tex.pixels.resize(static_cast<std::size_t>(tex.width) * static_cast<std::size_t>(tex.height));
        const auto* srcPixels = static_cast<const std::uint32_t*>(surface->pixels);
        const int step = surface->pitch / static_cast<int>(sizeof(std::uint32_t));

        for (int y = 0; y < tex.height; ++y) {
            for (int x = 0; x < tex.width; ++x) {
                tex.pixels[static_cast<std::size_t>(y) * tex.width + x] = srcPixels[y * step + x];
            }
        }
        textures_[id] = std::move(tex);
    }

    const Texture& TextureStore::get(int id) const {
        static const Texture emptyTexture;
        auto it = textures_.find(id);
        if (it != textures_.end()) {
            return it->second;
        }
        return emptyTexture;
    }
}