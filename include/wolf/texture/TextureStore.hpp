#pragma once

#include <map>
#include <string>

#include "wolf/texture/Texture.hpp"

namespace wolf {

    class TextureStore {
    public:
        void load(int id, const std::string& filename);

        const Texture& get(int id) const;

    private:
        std::map<int, Texture> textures_;
    };

}
