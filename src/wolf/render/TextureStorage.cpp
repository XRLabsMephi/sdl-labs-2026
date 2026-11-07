#include "render/TextureStorage.h"
#include "render/TextureStorage.h"

#include <ranges>

#include "debug/Logger.h"

namespace {
    constexpr const char* component = "TEX STRG";
}

namespace render {
    TextureStorage::TextureStorage(size_t size) : max_id_(size - 1), textures_(size, Texture::empty){
    }

    void TextureStorage::add(Texture texture, size_t id) {
        if (id >= textures_.size()) {
            throw std::out_of_range(
                std::format("texture id {} out of range (size {})", id, textures_.size()));
        }
        if (used_[id]) {
            throw std::invalid_argument(std::format("duplicate id {} assigning", id));
        }
        used_[id] = true;
        textures_[id] = std::move(texture);
    }

    void TextureStorage::add(std::vector<std::pair<Texture, size_t>> textures) {
        for (auto &[texture, id]: textures) {
            add(std::move(texture), id);
        }
    }

    void TextureStorage::addFromFile(const std::string &filename, const size_t id) {
        add(loadOrEmpty(filename), id);
    }

    void TextureStorage::addFromEntries(const std::vector<TextureEntry> &entries) {
        for (auto& [filename, id]: entries) {
            add(loadOrEmpty(filename), id);
        }
    }

    void TextureStorage::reset(size_t size) {
        textures_.assign(size, Texture::empty);
        used_.assign(size, false);
    }

    void TextureStorage::resize(size_t size) {
        max_id_ = size - 1;
        textures_.resize(size, Texture::empty);
        used_.resize(size);
    }

    bool TextureStorage::contains(size_t id) const noexcept { return id < used_.size() && used_[id]; }

    const Texture& TextureStorage::get(size_t id) {
        return textures_[id];
    }


    const Texture& TextureStorage::getOrEmpty(size_t id) noexcept {
        return id > max_id_ ? Texture::empty : textures_[id];
    }

    Texture TextureStorage::loadOrEmpty(const std::string &filename) {
        try {
            Texture tmp(filename);
            return tmp;
        } catch (const std::exception & ex) {
            logging::error(
                component,
                1,
                std::format("failed to load texture: {}, using placeholder", ex.what()),
                ""
                );
            return Texture::empty;
        }
    }
}
