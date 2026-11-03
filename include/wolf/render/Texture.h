#pragma once
#include <string>
#include <vector>
#include <SDL3/SDL_stdinc.h>

#include "core/Color.h"

namespace render {
    class TextureStorage;
}

namespace render {
    struct Texture {
        int width = 0, height = 0;
        std::vector<Uint32> pixels;

        Texture() = default;

        explicit Texture(const std::string &filename);

        static Texture createFromFile(const std::string &filename);

        [[nodiscard]] Uint32 pixel(int x, int y) const { return pixels[x * height + y]; }

        Uint32 *column(int x){ return &pixels[x * height]; }

        static const Texture empty;

    private:
        Texture(int w, int h, core::Color color)
            : width(w)
              , height(h)
              , pixels(static_cast<size_t>(width) * height, color.toUint32_t()) {
        }
    };

    inline const Texture Texture::empty = Texture(512, 512, core::Color::green);
}
