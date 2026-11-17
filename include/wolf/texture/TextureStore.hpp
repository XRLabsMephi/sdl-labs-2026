#pragma once
#include "wolf/texture/Texture.hpp"
#include <string>
#include <unordered_map>

class TextureStore {
private:
    std::unordered_map<int, Texture> textures;
public:
    bool load(int id, const std::string& filename);
    const Texture* get(int id) const;
};