#pragma once
#include "TextureStorage.h"

namespace render {
    namespace fs = std::filesystem;

    class TextureStorageLoader {
        fs::path mapping_file_path_;

        static std::vector<TextureStorage::TextureEntry> parseMapping(std::istream &stream, const fs::path &base_dir);

    public:
        explicit TextureStorageLoader(fs::path mapping_file_path);

        void load(TextureStorage &storage) const;
    };
}
