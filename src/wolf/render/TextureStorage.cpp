#include "render/TextureStorage.h"

#include "debug/Logger.h"

namespace {
    auto component = "TEX STRG";
}

size_t TextureStorage::add(Texture texture) {
    textures_.push_back(std::move(texture));
    return textures_.size() - 1;
}

size_t TextureStorage::add(const std::string &filename) {
    try {
        textures_.emplace_back(filename);
        return textures_.size() - 1;
    } catch (const std::exception &ex) {
        logging::error(
            component,
            1,
            std::format("texture \"{}\" creation failed with error: {}", filename, ex.what()),
            "");
        throw;
    }
}

std::vector<size_t> TextureStorage::add(const std::vector<std::string> &filenames) {
    size_t errors_count = 0;
    std::vector<size_t> indexes;
    indexes.reserve(filenames.size());
    for (const auto &filename: filenames) {
        try {
            indexes.emplace_back(add(filename));
        } catch (const std::exception&) {
            ++errors_count;
        }
    }
    if (errors_count > 0) {
        logging::warning(component,std::format("adding finished with {} errors", errors_count));
    }

    return indexes;
}

std::vector<size_t> TextureStorage::add_from(const std::string &directory_name, bool recursive) {
    namespace fs = std::filesystem;
    fs::path directory = fs::current_path() / directory_name;
    std::vector<size_t> indexes;

    size_t errors_count = 0;
    using rec_dir_it = fs::recursive_directory_iterator;
    try {
        for (auto it = rec_dir_it(directory); it != rec_dir_it(); ++it) {
            if (!recursive) {
                it.disable_recursion_pending();
            }

            try {
                const auto &entry = *it;

                if (entry.is_regular_file() && entry.path().extension() == ".png") {
                    fs::path relative_path = entry.path().lexically_relative(fs::current_path());
                    indexes.emplace_back(add(relative_path.string()));
                }
            }catch (const fs::filesystem_error& error) {
                logging::warning(
                    component,
                    std::format("skipping entry due to filesystem error: {}", error.what())
                    );
            } catch (const std::exception&) {
                ++errors_count;
            }
        }
    } catch (const fs::filesystem_error &e) {
        logging::error(
            component,
            1,
            std::format("adding from directory failed with filesystem error: {}", e.what()),
            format("directory = \"{}\", recursive = {}", directory_name, recursive)
        );
        throw;
    }

    if (errors_count > 0) {
        logging::warning(
            component,
            std::format("adding finished with {} errors", errors_count)
            );
    }

    return indexes;
}

Texture & TextureStorage::getTexture(size_t id) {
    return textures_[id];
}

void TextureStorage::fill(const std::vector<std::string> &filenames) {
    textures_.clear();
    textures_.reserve(filenames.size());
    add(filenames);
}

void TextureStorage::fill(const std::string &directory_name, bool recursive) {
    textures_.clear();
    add_from(directory_name, recursive);
}
