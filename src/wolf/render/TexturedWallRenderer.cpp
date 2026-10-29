#include "wolf/render/TexturedWallRenderer.hpp"

#include "wolf/render/FrameBuffer.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace wolf {
namespace {

constexpr Color ceilingColor = makeColor(58, 65, 82);
constexpr Color floorColor = makeColor(63, 56, 52);

[[nodiscard]] Color shadeHorizontalFace(Color color) noexcept {
    return makeColor(
        static_cast<std::uint8_t>(((color >> 24) & 0xffu) * 3u / 4u),
        static_cast<std::uint8_t>(((color >> 16) & 0xffu) * 3u / 4u),
        static_cast<std::uint8_t>(((color >> 8) & 0xffu) * 3u / 4u),
        static_cast<std::uint8_t>(color & 0xffu));
}

} // namespace

void TexturedWallRenderer::drawFrame(FrameBuffer& frame,
                                     std::span<const WallSlice> columns,
                                     const TextureLookup& textures,
                                     const FogSettings& fog) {
    if (columns.size() != static_cast<std::size_t>(frame.width())) {
        throw std::invalid_argument("Wall column count must equal framebuffer width");
    }
    frame.clear(floorColor);
    frame.fillRows(0, frame.height() / 2, ceilingColor);
    for (int x = 0; x < frame.width(); ++x) {
        const WallSlice& slice = columns[static_cast<std::size_t>(x)];
        if (!slice.hit.hit || slice.hit.wallId <= 0 || slice.drawEnd <= slice.drawStart) {
            continue;
        }
        const Texture& texture = textures(slice.hit.wallId);
        if (texture.width <= 0 || texture.height <= 0) {
            throw std::invalid_argument("Wall texture has invalid dimensions");
        }
        const int textureX = wallTextureX(slice.hit.wallU, texture.width);
        const int yStart = std::max(0, slice.drawStart);
        const int yEnd = std::min(frame.height(), slice.drawEnd);
        for (int y = yStart; y < yEnd; ++y) {
            const int textureY = wallTextureY(slice, y, frame.height(), texture.height);
            Color color = texture.sample(textureX, textureY);
            if (!isXSide(slice.hit.side)) {
                color = shadeHorizontalFace(color);
            }
            // Walls are opaque, regardless of the input BMP alpha channel.
            color = (color & 0xffffff00u) | 0xffu;
            frame.setPixel(x, y, applyFog(color, slice.hit.depth, fog));
        }
    }
}

} // namespace wolf
