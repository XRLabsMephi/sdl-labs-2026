#pragma once
#include "TextureStorage.h"

namespace render {
    namespace fs = std::filesystem;

    class TextureStorageInflator {
        fs::path mapping_file_path_;

        static std::vector<TextureStorage::TextureEntry> parseMapping(std::istream &stream, const fs::path &base_dir);

    public:
        explicit TextureStorageInflator(fs::path mapping_file_path);

        void inflate(TextureStorage &storage) const;
    };
}
