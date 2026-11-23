#pragma once

#include "wolf/render/Texture.hpp"

#include <map>

namespace wolf
{

class TextureStore
{
public:
    // Magenta is transparent when loading a sprite.
    void loadBmp(int id, const char* path, bool colorKey = false);

    [[nodiscard]] const Texture& at(int id) const
    {
        return textures_.at(id);
    }

private:
    std::map<int, Texture> textures_;
};

} // namespace wolf
