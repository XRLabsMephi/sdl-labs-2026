#pragma once

#include "wolf/texture/Texture.hpp"

#include <filesystem>
#include <unordered_map>

namespace wolf {

class TextureStore {
public:
    void load(int id, const std::filesystem::path& path);
    [[nodiscard]] const Texture& get(int id) const;
    [[nodiscard]] bool contains(int id) const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;

private:
    std::unordered_map<int, Texture> textures_;
};

} // namespace wolf
