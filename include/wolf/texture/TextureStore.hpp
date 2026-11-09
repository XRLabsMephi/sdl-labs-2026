#pragma once

#include <filesystem>
#include <unordered_map>

#include "wolf/texture/Texture.hpp"

namespace texture {
    // Minimal resource contract for Application / participant 1.
    // File loading accepts PNG only and converts it to the CPU renderer's packed RGBA.
    class TextureStore {
    public:
        // Material opacity multiplies the PNG alpha (255 keeps the original alpha).
        void load(int id, const std::filesystem::path& path, std::uint8_t opacity = 255);
        void set(int id, Texture texture, std::uint8_t opacity = 255);
        [[nodiscard]] const Texture* get(int id) const noexcept;
        [[nodiscard]] bool isOpaque(int id) const noexcept;

    private:
        struct Entry {
            Texture texture;
            bool opaque;
        };
        std::unordered_map<int, Entry> textures_;
    };
}
