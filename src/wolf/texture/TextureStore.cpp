#include "wolf/texture/TextureStore.hpp"

#include <SDL3/SDL.h>

#include <cstddef>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

namespace wolf {
namespace {

using Surface = std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)>;

std::runtime_error resourceError(const std::filesystem::path& path, const char* operation) {
    return std::runtime_error("Texture '" + path.string() + "': " + operation + ": " + SDL_GetError());
}

struct SurfaceUnlock {
    SDL_Surface* surface;
    ~SurfaceUnlock() { SDL_UnlockSurface(surface); }
};

Texture loadBmp(const std::filesystem::path& path) {
    const Surface original(SDL_LoadBMP(path.string().c_str()), SDL_DestroySurface);
    if (!original) { throw resourceError(path, "SDL_LoadBMP"); }
    const Surface converted(SDL_ConvertSurface(original.get(), SDL_PIXELFORMAT_RGBA8888),
                            SDL_DestroySurface);
    if (!converted) { throw resourceError(path, "SDL_ConvertSurface RGBA8888"); }

    Texture texture;
    texture.width = converted->w;
    texture.height = converted->h;
    const auto width = static_cast<std::size_t>(texture.width);
    const auto height = static_cast<std::size_t>(texture.height);
    if (texture.width <= 0 || texture.height <= 0 ||
        width > texture.pixels.max_size() / height || converted->pitch <= 0 ||
        static_cast<std::size_t>(converted->pitch) < width * sizeof(Color)) {
        throw std::runtime_error("Texture '" + path.string() + "': invalid surface dimensions or pitch");
    }
    texture.pixels.resize(width * height);
    if (!SDL_LockSurface(converted.get())) { throw resourceError(path, "SDL_LockSurface"); }
    const SurfaceUnlock unlock{converted.get()};
    const auto* pixels = static_cast<const std::byte*>(converted->pixels);
    for (std::size_t y = 0; y < height; ++y) {
        // Packed integers retain 0xRRGGBBAA on either host byte order.
        std::memcpy(texture.pixels.data() + y * width,
                    pixels + y * static_cast<std::size_t>(converted->pitch), width * sizeof(Color));
    }
    return texture;
}

} // namespace

void TextureStore::load(int id, const std::filesystem::path& path) {
    if (id <= 0) { throw std::invalid_argument("Texture ID must be positive: " + std::to_string(id)); }
    if (contains(id)) { throw std::invalid_argument("Duplicate texture ID: " + std::to_string(id)); }
    Texture texture = loadBmp(path);
    textures_.emplace(id, std::move(texture));
}

bool TextureStore::contains(int id) const noexcept {
    return textures_.contains(id);
}

const Texture& TextureStore::get(int id) const {
    const auto found = textures_.find(id);
    if (found == textures_.end()) {
        throw std::out_of_range("Unknown texture ID: " + std::to_string(id));
    }
    return found->second;
}

} // namespace wolf
