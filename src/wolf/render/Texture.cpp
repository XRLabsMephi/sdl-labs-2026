#include "render/Texture.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <format>
#include <memory>
#include <filesystem>
#include <SDL3/SDL_surface.h>
#include <iostream>

#include "debug/Logger.h"

namespace {
    struct SurfaceDeleter {
        void operator ()(SDL_Surface* surface) const {
            SDL_DestroySurface(surface);
        }
    };

    const char* utf8_as_chars(const std::u8string& s) noexcept {
        return reinterpret_cast<const char*>(s.c_str());
    }
}

namespace render {
    Texture::Texture(const std::filesystem::path& file) {
        namespace fs = std::filesystem;
        using SurfacePtr =  std::unique_ptr<SDL_Surface, SurfaceDeleter>;

        std::string ext = file.extension().string();
        std::ranges::transform(
            ext,
            ext.begin(),
            [](const unsigned char c) { return static_cast<char>(std::tolower(c)); }
            );

        if (ext != ".png") {
            throw std::runtime_error(
                std::format("file \"{}\" must have .png extension", utf8_as_chars(file.u8string()))
                );
        }

        SurfacePtr loaded(SDL_LoadPNG(utf8_as_chars(file.u8string())));
        if (!loaded) {
            throw std::runtime_error(
                std::format(
                    "SDL_Surface creation from file \"{}\" failed: {}",
                    utf8_as_chars(file.u8string()),
                    SDL_GetError())
                );
        }

        SurfacePtr surface(SDL_ConvertSurface(loaded.get(), SDL_PIXELFORMAT_ARGB8888));
        if (!surface) {
            throw std::runtime_error(
                std::format(
                    "SDL_Surface from file \"{}\" conversion to ARGB8888 failed: {}",
                    utf8_as_chars(file.u8string()),
                    SDL_GetError()
                    )
                );
        }

        //на будущее
        /*if (!SDL_PremultiplySurfaceAlpha(surface.get(), false)) {
            throw std::runtime_error(
                std::format(
                    "SDL_Surface from file \"{}\" premultiplying failed: {}", filename, SDL_GetError()
                    )
                );
        }*/

        width = surface->w;
        height = surface->h;
        pixels.resize(static_cast<size_t>(width) * height);

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

    Texture Texture::createFromFile(const std::filesystem::path& file){
        return Texture(file);
    }
}