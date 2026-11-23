#include "wolf/texture/TextureStore.hpp"

#include <limits>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <png.h>

#include "wolf/core/color.hpp"

namespace texture {
    void TextureStore::load(int id, const std::filesystem::path &path, uint8_t opacity) {
        const int MAX_IMAGE_DIM = std::numeric_limits<int>::max();
        png_image image{};
        Texture texture;
        image.version = PNG_IMAGE_VERSION;
        std::vector<png_byte> data;
        const std::string filename = path.string();
        const char* err = nullptr;


        if (!png_image_begin_read_from_file(&image, filename.c_str())) {
            err = "Cannot load PNG";
            goto cleanup;
        }

        if (image.width > (png_uint_32) (MAX_IMAGE_DIM) ||
            image.height > (png_uint_32) (MAX_IMAGE_DIM)) {
            err = "PNG dimensions are too large";
            goto cleanup;
        }

        try {
            image.format = PNG_FORMAT_RGBA;
            data.resize(PNG_IMAGE_SIZE(image));
            if (!png_image_finish_read(&image, nullptr, data.data(), 0, nullptr)) {
                err = "Cannot decode PNG";
                goto cleanup;
            }

            texture.width = image.width;
            texture.height = image.height;
            texture.pixels.resize(texture.width * texture.height);
            for (size_t i = 0; i < texture.pixels.size(); ++i) {
                size_t offset = i * 4;
                texture.pixels[i] = core::ColorRGBA::packColor(
                    data[offset], data[offset + 1], data[offset + 2], data[offset + 3]);
            }
        } catch (const std::bad_alloc &) {
            err = "Out of memory";
            goto cleanup;
        }
    cleanup:
        png_image_free(&image);
        if (err != nullptr) {
            throw std::runtime_error(std::string(err) + ": " + filename);
        }
        set(id, std::move(texture), opacity);
    }

    void TextureStore::set(int id, Texture texture, std::uint8_t opacity) {
        if (!texture.valid()) {
            throw std::invalid_argument("Texture dimensions must match its pixel data");
        }
        if (opacity != 255) {
            for (auto &color : texture.pixels) {
                auto alpha = ((color >> 24U) * opacity + 127U) / 255U;
                color = (color & 0x00FFFFFFU) | (alpha << 24U);
            }
        }

        bool opaque = true;
        for (const auto &color : texture.pixels) {
            if ((color >> 24) != 255) {
                opaque = false;
                break;
            }
        }

        textures_[id] = {std::move(texture), opaque};
    }

    const Texture *TextureStore::get(int id) const noexcept {
        auto it = textures_.find(id);
        if (it == textures_.end()) {
            return nullptr;
        }
        return &it->second.texture;
    }

    bool TextureStore::isOpaque(int id) const noexcept {
        auto it = textures_.find(id);
        if (it == textures_.end()) {
            return true;
        }
        return it->second.opaque;
    }
}
