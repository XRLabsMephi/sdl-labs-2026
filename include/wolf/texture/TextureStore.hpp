#pragma once

#include "wolf/texture/Texture.hpp"

#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

namespace wolf {

class TextureStore {
public:
    void load(int id, const std::filesystem::path& path,
              std::optional<std::uint32_t> colorKey = std::nullopt);

    void set(int id, Texture texture) {
        if (id <= 0 || texture.width <= 0 || texture.height <= 0
            || texture.pixels.size() != static_cast<std::size_t>(texture.width) * texture.height) {
            throw std::invalid_argument("Некорректная текстура");
        }
        textures_.insert_or_assign(id, std::move(texture));
    }

    const Texture& get(int id) const {
        const auto found = textures_.find(id);
        if (found == textures_.end()) {
            throw std::out_of_range("Не найдена текстура: " + std::to_string(id));
        }
        return found->second;
    }

private:
    std::unordered_map<int, Texture> textures_;
};

}
