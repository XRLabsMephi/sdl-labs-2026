#pragma once

#include "wolf/texture/Texture.hpp"

#include <optional>
#include <string>
#include <unordered_map>

namespace wolf
{

class TextureStore
{
public:
    void insert(int textureId, Texture texture);
    void loadPpm(int textureId, const std::string& path,
                 std::optional<Texture::Pixel> transparentColor = std::nullopt);

    [[nodiscard]] bool contains(int textureId) const;
    [[nodiscard]] const Texture& get(int textureId) const;

private:
    std::unordered_map<int, Texture> textures_;
};

} // namespace wolf
