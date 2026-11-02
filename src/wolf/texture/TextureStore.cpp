#include "wolf/texture/TextureStore.hpp"

#include "wolf/texture/BmpLoader.hpp"

#include <stdexcept>
#include <string>
#include <utility>

namespace wolf {

    void TextureStore::load(int id, const std::filesystem::path& path) {
        if (id <= 0) {
            throw std::invalid_argument("Texture ID must be positive: " + std::to_string(id));
        }
        if (contains(id)) {
            throw std::invalid_argument("Duplicate texture ID: " + std::to_string(id));
        }
        Texture texture;
        texture.pixels = loadBmpPixels(path, texture.width, texture.height);
        textures_.emplace(id, std::move(texture));
    }

    const Texture& TextureStore::get(int id) const {
        const auto found = textures_.find(id);
        if (found == textures_.end()) {
            throw std::out_of_range("Unknown texture ID: " + std::to_string(id));
        }
        return found->second;
    }

    bool TextureStore::contains(int id) const noexcept {
        return textures_.contains(id);
    }

    std::size_t TextureStore::size() const noexcept {
        return textures_.size();
    }

}
