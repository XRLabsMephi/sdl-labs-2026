#pragma once

#include <filesystem>
#include <unordered_map>

#include "wolf/texture/Texture.hpp"

namespace texture {
    class TextureStore {
    public:
        void load(int id, const std::filesystem::path& path, uint8_t opacity = 255);
        void set(int id, Texture texture, std::uint8_t opacity = 255);
        [[nodiscard]] const Texture *get(int id) const noexcept;
        bool isOpaque(int id) const noexcept;

    private:
        struct Entry {
            Texture texture;
            bool opaque;
        };
        std::unordered_map<int, Entry> textures_;
    };
}
