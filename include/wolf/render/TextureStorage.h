#pragma once
#include <filesystem>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Texture.h"

namespace render {
    class TextureStorage {
        size_t max_id_ = 0;
        std::vector<Texture> textures_;
        std::unordered_set<size_t> used;

        /**
         * @brief Tries to load a Texture.
         * @param filename name of the file
         * @return loaded Texture or placeholder on error
         */
        static Texture loadOrEmpty(const std::string &filename);

    public:
        /**
         * @brief Specifies the name of the file and id of the texture in the storage
         */
        struct TextureEntry {
            std::string filename;
            size_t id = 0;
        };

        /**
         * @brief Creates empty texture storage
         */
        TextureStorage() = default;

        /**
         * Creates texture storage of given size
         * @param size number of textures to store
         */
        explicit TextureStorage(size_t size);

        /**
         * @brief Associates given texture with given id in this storage
         * @param texture
         * @param id
         */
        void add(Texture texture, size_t id);

        void add(std::vector<std::pair<Texture, size_t> > textures);

        /**
         * Loads texture from given file into storage
         * @param filename the name of the texture file
         * @param id the id in the storage
         */
        void addFromFile(const std::string& filename, size_t id);

        /**
         * @brief Loads textures from given entries into storage
         * @param entries entries to add from
         */
        void addFromEntries(const std::vector<TextureEntry>& entries);

        /**
         * @brief Resizes the storage, filling new slots with placeholders
         * @param size new size of storage
         */
        void resize(size_t size);

        /**
         * @param id id of texture
         * @return const reference to Texture with given id or const reference to a placeholder
         */
        inline const Texture& getOrEmpty(size_t id) noexcept;

        /**
         * @return max ID used by this storage
         */
        size_t maxId() const { return max_id_; }

        /**
         * @return The size of the storage
         */
        size_t size() const { return textures_.size(); }
    };
}
