#include "wolf/texture/TextureStore.hpp"
#include <SDL3/SDL.h>

bool TextureStore::load(int id, const std::string& filename) {
    SDL_Surface* surface = SDL_LoadBMP(filename.c_str());
    if (surface == nullptr) return false;
    SDL_Surface* converted = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_ARGB8888);
    SDL_DestroySurface(surface);
    if (converted == nullptr) return false;
    if (SDL_MUSTLOCK(converted) && !SDL_LockSurface(converted)) {
        SDL_DestroySurface(converted);
        return false;
    }
    Texture texture;
    texture.width = converted->w;
    texture.height = converted->h;
    texture.pixels.resize(texture.width * texture.height);
    for (int y = 0; y < texture.height; y++) {
        const auto* row = reinterpret_cast<const Color*>(
            static_cast<const unsigned char*>(converted->pixels) + y * converted->pitch
        );
        for (int x = 0; x < texture.width; x++) {
            texture.pixels[y * texture.width + x] = row[x];
        }
    }
    if (SDL_MUSTLOCK(converted)) SDL_UnlockSurface(converted);
    SDL_DestroySurface(converted);
    textures[id] = std::move(texture);
    return true;
}

const Texture* TextureStore::get(int id) const {
    auto it = textures.find(id);
    if (it == textures.end()) return nullptr;
    return &it->second;
}