#include "render/Texture.h"

#include <cstring>
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

namespace render {
    Texture::Texture(const std::string &filename) {
        namespace fs = std::filesystem;
        using SurfacePtr =  std::unique_ptr<SDL_Surface, SurfaceDeleter>;
        fs::path path = filename;

        std::string ext = path.extension().string();
        std::ranges::transform(ext, ext.begin(),
        [](const unsigned char c) { return static_cast<char>(std::tolower(c)); });

        //TODO Add support for JPG, Webp, and others
        if (ext != ".png") {
            throw std::runtime_error(std::format("file \"{}\" must have .png extension", filename));
        }

        SurfacePtr loaded(SDL_LoadPNG(path.string().c_str()));
        if (!loaded) {
            throw std::runtime_error(
                std::format("SDL_Surface creation from file \"{}\" failed: {}", filename, SDL_GetError())
                );
        }

        SurfacePtr surface(SDL_ConvertSurface(loaded.get(), SDL_PIXELFORMAT_RGBA8888));
        if (!surface) {
            throw std::runtime_error(
                std::format(
                    "SDL_Surface from file \"{}\" conversion to RGBA8888 failed: {}", filename, SDL_GetError()
                    )
                );
        }

        width = surface->w;
        height = surface->h;
        pixels.resize(static_cast<size_t>(width) * height);

        //TODO implement tiling loading
        const auto *base = static_cast<const Uint8 *>(surface->pixels);
        for (int y = 0; y < height; ++y) {
            const Uint8 *row = base + static_cast<size_t>(y) * surface->pitch;

            for (int x = 0; x < width; ++x) {
                Uint32 value;
                std::memcpy(&value, row + static_cast<size_t>(x) * sizeof(Uint32), sizeof(value));
                pixels[static_cast<size_t>(x) * height + y] = value;
            }
        }
    }

    Texture Texture::createFromFile(const std::string &filename){
        return Texture(filename);
    }
}