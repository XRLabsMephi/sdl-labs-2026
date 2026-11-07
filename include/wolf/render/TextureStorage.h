#pragma once
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include "Texture.h"

namespace render {
    class TextureStorage {
        size_t max_id_ = 0;
        std::vector<Texture> textures_;
        std::vector<bool> used_;

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
         * @brief Resets the storage with the given size
         * @param size the size of reset storage
         */
        void reset(size_t size);

        /**
         * @brief Resizes the storage, filling new slots with placeholders
         * @param size new size of storage
         */
        void resize(size_t size);

        /**
         * Checks if given id is used
         * @return true if id is used by some texture
         */
        [[nodiscard]] bool contains(size_t id) const noexcept;

        /**
         * @param id id of texture
         * @return const reference to Texture with given id.
         * @attention Not protected from invalid ids
         */
        inline const Texture& get(size_t id);

        /**
         * @param id id of texture
         * @return const reference to Texture with given id or const reference to a placeholder
         */
        inline const Texture& getOrEmpty(size_t id) noexcept;

        /**
         * @return max ID used by this storage
         */
        [[nodiscard]] size_t maxId() const { return max_id_; }

        /**
         * @return The size of the storage
         */
        [[nodiscard]] size_t size() const { return textures_.size(); }
    };
}
