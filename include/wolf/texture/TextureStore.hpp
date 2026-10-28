#pragma once

#include "wolf/texture/Texture.hpp"

#include <filesystem>
#include <unordered_map>

namespace wolf {

class TextureStore {
public:
    // IDs are positive and cannot be overwritten by a second resource.
    void load(int id, const std::filesystem::path& path);
    [[nodiscard]] bool contains(int id) const noexcept;
    [[nodiscard]] const Texture& get(int id) const;

private:
    std::unordered_map<int, Texture> textures_;
};

} // namespace wolf
