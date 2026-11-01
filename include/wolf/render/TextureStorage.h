#pragma once
#include <filesystem>
#include <string>
#include <vector>

#include "Texture.h"

class TextureStorage {
    std::vector<Texture> textures_;

public:
    /**
     * @brief Creates empty texture storage
     */
    TextureStorage() = default;

    /**
     * Fills storage with Textures created from filenames
     * @param filenames files to create textures from
     */
    void fill(const std::vector<std::string> &filenames);

    /**
     * Fills storage with Textures created from all files in directory
     * @param directory_name directory to search for
     * @param recursive should go into subdirectories
     */
    void fill(const std::string &directory_name, bool recursive = true);

    size_t add(Texture texture);

    size_t add(const std::string& filename);

    std::vector<size_t> add(const std::vector<std::string> &filenames);

    std::vector<size_t> add_from(const std::string &directory_name, bool recursive = true);

    Texture& getTexture(size_t id);
};
