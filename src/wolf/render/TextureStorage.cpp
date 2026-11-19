#include "render/TextureStorage.h"

#include <ranges>

#include "debug/Logger.h"

namespace {
    constexpr const char* component = "TEX STRG";
}

namespace render {
    TextureStorage::TextureStorage(size_t size){
        reset(size);
    }

    void TextureStorage::add(Texture texture, size_t id) {
        if (id >= textures_.size()) {
            throw logging::Error(logging::ErrorCode::TextureId,
                std::format("texture id {} out of range (size {})", id, textures_.size()));
        }
        if (used_[id]) {
            throw logging::Error(logging::ErrorCode::TextureDuplicate, std::format("duplicate id {} assigning", id));
        }
        used_[id] = true;
        textures_[id] = std::move(texture);
    }

    void TextureStorage::add(const std::vector<std::pair<Texture, size_t>>& textures) {
        for (auto &[texture, id]: textures) {
            add(texture, id);
        }
    }

    void TextureStorage::addFromFile(const std::string &filename, const size_t id) {
        if (std::optional<Texture> texture = loadOrEmpty(filename)) add(texture.value(), id);
    }

    void TextureStorage::addFromEntries(const std::vector<TextureEntry> &entries) {
        for (auto& [filename, id]: entries) {
            if (std::optional<Texture> texture = loadOrEmpty(filename)) add(texture.value(), id);
        }
    }

    void TextureStorage::reset(size_t size) {
        max_id_ = size == 0 ? 0 : size - 1;
        textures_.assign(size, std::nullopt);
        used_.assign(size, false);
    }

    void TextureStorage::resize(size_t size) {
        max_id_ = size == 0 ? 0 : size - 1;
        textures_.resize(size, std::nullopt);
        used_.resize(size);
    }

    bool TextureStorage::contains(size_t id) const noexcept { return id < used_.size() && used_[id]; }

    const Texture& TextureStorage::get(size_t id) const{
        return textures_[id].value();
    }


    const Texture& TextureStorage::getOrEmpty(size_t id) const noexcept {
        return contains(id) && textures_[id].has_value() ? textures_[id].value() : Texture::empty;
    }

    std::optional<Texture> TextureStorage::loadOrEmpty(const std::string &filename) {
        try {
            Texture tmp(filename);
            return tmp;
        } catch (const std::exception & ex) {
            logging::error(
                component,
                logging::ErrorCode::TextureRead,
                std::format("failed to load texture: {}, using placeholder", ex.what()),
                ""
                );
            return std::nullopt;
        }
    }
}
