#include "wolf/texture/TextureStore.hpp"

#include <SDL3/SDL.h>

#include <cstring>
#include <memory>
#include <stdexcept>
#include <utility>

namespace wolf {

void TextureStore::load(int id, const std::string& filename) {
    if (id <= 0) {
        throw std::runtime_error("Texture id must be positive");
    }

    if (textures_.contains(id)) {
        throw std::runtime_error("Texture id already exists: " + std::to_string(id));
    }

    using SurfacePtr = std::unique_ptr<SDL_Surface, void (*)(SDL_Surface*)>;

    SurfacePtr original(
        SDL_LoadBMP(filename.c_str()),
        SDL_DestroySurface
    );

    if (!original) {
        throw std::runtime_error("Cannot load texture: " + filename + ": " + SDL_GetError());
    }

    SurfacePtr surface(
        SDL_ConvertSurface(original.get(), SDL_PIXELFORMAT_RGBA32),
        SDL_DestroySurface
    );

    if (!surface) {
        throw std::runtime_error("Cannot convert texture: " + filename + ": " + SDL_GetError());
    }

    if (surface->w <= 0 || surface->h <= 0) {
        throw std::runtime_error("Invalid texture size: " + filename);
    }

    Texture texture;
    texture.width = surface->w;
    texture.height = surface->h;

    texture.pixels.resize(
        static_cast<std::size_t>(texture.width) * texture.height
    );

    if (!SDL_LockSurface(surface.get())) {
        throw std::runtime_error(
            "Cannot access texture pixels: " +
            filename + ": " + SDL_GetError()
        );
    }

    const auto* pixels = static_cast<const unsigned char*>(surface->pixels);

    for (int y = 0; y < texture.height; ++y) {
        std::memcpy(
            texture.pixels.data() +
                static_cast<std::size_t>(y) * texture.width,

            pixels +
                static_cast<std::size_t>(y) * surface->pitch,

            static_cast<std::size_t>(texture.width) * sizeof(Color)
        );
    }

    SDL_UnlockSurface(surface.get());

    textures_.emplace(id, std::move(texture));
}

const Texture& TextureStore::get(int id) const {
    auto it = textures_.find(id);

    if (it == textures_.end()) {
        throw std::runtime_error("Unknown texture id: " + std::to_string(id));
    }

    return it->second;
}

}
