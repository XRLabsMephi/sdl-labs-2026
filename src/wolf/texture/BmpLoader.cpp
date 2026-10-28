#include "wolf/texture/BmpLoader.hpp"

#include <SDL3/SDL.h>

#include <cstddef>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>

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

    }

    std::vector<Color> loadBmpPixels(const std::filesystem::path& path, int& width, int& height) {
        const Surface original(SDL_LoadBMP(path.string().c_str()), SDL_DestroySurface);
        if (!original) {
            throw resourceError(path, "SDL_LoadBMP");
        }
        const Surface converted(SDL_ConvertSurface(original.get(), SDL_PIXELFORMAT_RGBA8888),
                                SDL_DestroySurface);
        if (!converted) {
            throw resourceError(path, "SDL_ConvertSurface RGBA8888");
        }
        std::vector<Color> result;
        const auto w = static_cast<std::size_t>(converted->w);
        const auto h = static_cast<std::size_t>(converted->h);
        if (converted->w <= 0 ||
            converted->h <= 0 ||
            w > result.max_size() / h ||
            converted->pitch <= 0 ||
            static_cast<std::size_t>(converted->pitch) < w * sizeof(Color)) {
            throw std::runtime_error("Texture '" + path.string() + "': invalid surface dimensions or pitch");
        }
        result.resize(w * h);
        if (!SDL_LockSurface(converted.get())) {
            throw resourceError(path, "SDL_LockSurface");
        }
        const SurfaceUnlock unlock{converted.get()};
        const auto* pixels = static_cast<const std::byte*>(converted->pixels);
        for (std::size_t y = 0; y < h; ++y) {
            std::memcpy(result.data() + y * w,
                        pixels + y * static_cast<std::size_t>(converted->pitch),
                        w * sizeof(Color));
        }
        width = converted->w;
        height = converted->h;
        return result;
    }

}
