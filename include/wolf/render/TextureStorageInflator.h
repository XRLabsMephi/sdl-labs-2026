#pragma once
#include "TextureStorage.h"

namespace render {
    class TextureStorageInflator {
    public:
        TextureStorageInflator(const TextureStorageInflator &) = delete;
        TextureStorageInflator &operator =(const TextureStorageInflator &) = delete;

        static void inflateStorage(TextureStorage& storage, const std::string& mapping_file);
    };
}