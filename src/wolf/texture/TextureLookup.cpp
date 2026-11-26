#include "wolf/texture/TextureStore.hpp"

#include <stdexcept>

namespace wolf {

const Texture& TextureStore::get(int id) const {
    auto it = textures_.find(id);

    if (it == textures_.end()) {
        throw std::runtime_error("Unknown texture id: " + std::to_string(id));
    }

    return it->second;
}

}
