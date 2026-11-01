#include "render/Texture.h"

#include <filesystem>
#include <fstream>
#include <SDL3/SDL_surface.h>

#include "debug/Logger.h"

namespace {
    struct SurfaceDeleter {
        void operator ()(SDL_Surface* surface) const {
            SDL_DestroySurface(surface);
        }
    };
}

Texture Texture::createFromFile(const std::string &filename) {
    namespace fs = std::filesystem;
    fs::path path = fs::current_path() / filename;
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        logging::error("TEX STRG", 1, "failed to open texture file", "");
        throw std::runtime_error("failed to open texture file: " + filename);
    }

    logging::info("TEX STRG", "opened " + filename);
    std::string extension = path.extension().string();

    //TODO Add support for JPG, Webp, and others
    if (extension != ".png") {
        logging::error(
            "TEX STRG",
            1,
            "file \"" + filename + "\" must have .png extension", "");
        throw std::runtime_error("file \"" + filename + "\" must have .png extension");
    }

    std::unique_ptr<SDL_Surface, SurfaceDeleter> surface(SDL_LoadPNG(path.string().c_str()));
    if (!surface) {
        logging::error(
            "TEX STRG",
            1,
            "failed to create SDL_Surface from file \"" + filename + "\"", "");
        throw std::runtime_error("failed to open texture file: " + filename);
    }

    if (!SDL_LockSurface(surface.get())) {
        logging::error("TEX STRG", 1, "failed to lock SDL_Surface", "");
        throw std::runtime_error("failed to lock SDL_Surface while creating texture");
    }

    const int tex_w = surface->w;
    const int tex_h = surface->h;
    Texture texture{
        .width = tex_w,
        .height = tex_h,
        .pixels = std::vector(tex_w * tex_h, static_cast<Uint32>(0))
    };
    Uint32* tex_pixels = texture.pixels.data();

    const SDL_PixelFormatDetails* details = SDL_GetPixelFormatDetails(SDL_PIXELFORMAT_RGBA8888);

    size_t pixel_reading_errors = 0;
    for (int x = 0; x < tex_w; ++x) {
        for (int y = 0; y < tex_h; ++y) {
            Uint8 r, g, b, a;

            if (!SDL_ReadSurfacePixel(surface.get(), x, y, &r, &g, &b, &a)) {
                ++pixel_reading_errors;
                continue;
            }

            Uint32 pixel_value = SDL_MapRGBA(details, nullptr, r, g, b, a);

            tex_pixels[x * tex_h + y] = pixel_value;
        }
    }

    SDL_UnlockSurface(surface.get());

    if (pixel_reading_errors > 0) {
        logging::warning(
            "TEX STRG",
            std::format("failed to read {} pixels while loading texture \"{}\"",
                pixel_reading_errors, filename));
    }


    return texture;
}
