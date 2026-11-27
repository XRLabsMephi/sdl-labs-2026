#pragma once
#include "wolf/texture/Texture.hpp"
#include <unordered_map>
#include <string>

namespace wolf::texture {
    class TextureStore {
     public:
        TextureStore() = default;

        void load(int id, const std::string& filepath);

        [[nodiscard]] const Texture& get(int id) const;
     private:
        std::unordered_map<int, Texture> textures_;
    };
}