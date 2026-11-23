#include "wolf/texture/TextureStore.hpp"

#include <stdexcept>
#include <utility>

namespace wolf
{

void TextureStore::insert(int textureId, Texture texture)
{
    if (textureId <= 0)
    {
        throw std::invalid_argument("Texture identifier must be positive");
    }
    if (!texture.valid())
    {
        throw std::invalid_argument("Cannot store an invalid texture");
    }
    textures_.insert_or_assign(textureId, std::move(texture));
}

bool TextureStore::contains(int textureId) const
{
    return textures_.contains(textureId);
}

const Texture& TextureStore::get(int textureId) const
{
    const auto iterator = textures_.find(textureId);
    if (iterator == textures_.end())
    {
        throw std::out_of_range("Unknown texture identifier");
    }
    return iterator->second;
}

} // namespace wolf
