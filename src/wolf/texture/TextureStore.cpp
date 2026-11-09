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
    void TextureStore::load(int id, const std::filesystem::path& path, std::uint8_t opacity) {
        // libpng validates the file signature; renaming BMP/JPEG to .png won't load it.
        struct Image {
            png_image value{};
            Image() { value.version = PNG_IMAGE_VERSION; }
            ~Image() { png_image_free(&value); }
        } image;

        const auto filename = path.string();
        if (!png_image_begin_read_from_file(&image.value, filename.c_str())) {
            throw std::runtime_error("Cannot load PNG '" + filename + "': " + image.value.message);
        }
        const auto& info = image.value;
        if (info.width > static_cast<png_uint_32>(std::numeric_limits<int>::max()) ||
            info.height > static_cast<png_uint_32>(std::numeric_limits<int>::max())) {
            throw std::runtime_error("PNG dimensions are too large: " + filename);
        }

        image.value.format = PNG_FORMAT_RGBA;
        std::vector<png_byte> rgba(PNG_IMAGE_SIZE(image.value));
        if (!png_image_finish_read(&image.value, nullptr, rgba.data(), 0, nullptr)) {
            throw std::runtime_error("Cannot decode PNG '" + filename + "': " + image.value.message);
        }

        Texture texture;
        texture.width = static_cast<int>(info.width);
        texture.height = static_cast<int>(info.height);
        texture.pixels.resize(static_cast<std::size_t>(texture.width) * texture.height);
        for (std::size_t i = 0; i < texture.pixels.size(); ++i) {
            const auto offset = i * 4;
            texture.pixels[i] = core::ColorRGBA::packColor(
                rgba[offset], rgba[offset + 1], rgba[offset + 2], rgba[offset + 3]);
        }
        // Replace only after successful decoding, retaining the old resource on failure.
        set(id, std::move(texture), opacity);
    }

    void TextureStore::set(int id, Texture texture, std::uint8_t opacity) {
        if (!texture.valid()) {
            throw std::invalid_argument("Texture dimensions must match its pixel data");
        }
        if (opacity != 255) {
            for (auto& color : texture.pixels) {
                const auto alpha = ((color >> 24U) * opacity + 127U) / 255U;
                color = (color & 0x00FFFFFFU) | (alpha << 24U);
            }
        }
        const bool opaque = std::all_of(texture.pixels.begin(), texture.pixels.end(),
                                       [](auto color) { return (color >> 24U) == 255; });
        textures_.insert_or_assign(id, Entry{std::move(texture), opaque});
    }

    const Texture* TextureStore::get(int id) const noexcept {
        const auto found = textures_.find(id);
        return found == textures_.end() ? nullptr : &found->second.texture;
    }

    bool TextureStore::isOpaque(int id) const noexcept {
        const auto found = textures_.find(id);
        // Missing textures use the raycaster's opaque fallback color.
        return found == textures_.end() || found->second.opaque;
    }
}
