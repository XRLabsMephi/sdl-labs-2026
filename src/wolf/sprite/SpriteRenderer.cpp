#include "wolf/sprite/SpriteRenderer.hpp"

#include "wolf/raycast/Raycaster.hpp"
#include "wolf/render/FrameBuffer.hpp"
#include "wolf/world/Player.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace wolf {
namespace {

struct VisibleSprite {
    const Sprite* sprite;
    CameraPoint camera;
};

[[nodiscard]] std::uint8_t component(Color color, unsigned shift) noexcept {
    return static_cast<std::uint8_t>((color >> shift) & 0xffu);
}

[[nodiscard]] Color over(Color foreground, Color background) noexcept {
    const unsigned srcAlpha = foreground & 0xffu;
    if (srcAlpha == 0u) { return background; }
    if (srcAlpha == 255u) { return foreground; }

    const unsigned dstAlpha = background & 0xffu;
    const unsigned alphaNumerator = 255u * srcAlpha + dstAlpha * (255u - srcAlpha);
    const auto blendChannel = [&](unsigned shift) -> std::uint8_t {
        if (alphaNumerator == 0u) { return 0u; }
        const unsigned numerator =
            static_cast<unsigned>(component(foreground, shift)) * srcAlpha * 255u +
            static_cast<unsigned>(component(background, shift)) * dstAlpha * (255u - srcAlpha);
        return static_cast<std::uint8_t>((numerator + alphaNumerator / 2u) / alphaNumerator);
    };
    return makeColor(blendChannel(24), blendChannel(16), blendChannel(8),
                     static_cast<std::uint8_t>((alphaNumerator + 127u) / 255u));
}

[[nodiscard]] int clipStart(double edge, int maxValue) noexcept {
    return static_cast<int>(std::clamp(std::ceil(edge - 0.5), 0.0,
                                       static_cast<double>(maxValue)));
}

[[nodiscard]] int texel(double screenPos, double left, double extent,
                        int textureSize) noexcept {
    const double u = std::clamp((screenPos + 0.5 - left) / extent, 0.0, 1.0);
    return std::clamp(static_cast<int>(u * textureSize), 0, textureSize - 1);
}

} // namespace

void SpriteRenderer::drawSprites(FrameBuffer& frame, const Player& player,
                                 std::span<const Sprite> sprites,
                                 std::span<const double> depthBuffer,
                                 const TextureLookup& textures,
                                 const FogSettings& fog) {
    if (depthBuffer.size() != static_cast<std::size_t>(frame.width())) {
        throw std::invalid_argument("Wall depth buffer must equal framebuffer width");
    }
    const double focal = focalLength(player, frame.width());
    if (!std::isfinite(focal) || focal <= 0.0) { return; }

    std::vector<VisibleSprite> visible;
    visible.reserve(sprites.size());
    for (const Sprite& sprite : sprites) {
        if (!(std::isfinite(sprite.scale) && sprite.scale > 0.0)) { continue; }
        const CameraPoint camera = toCameraSpace(player, sprite.position);
        if (!(camera.depth > 0.0 && std::isfinite(camera.depth) &&
              std::isfinite(camera.cameraX))) { continue; }
        visible.push_back({&sprite, camera});
    }
    std::stable_sort(visible.begin(), visible.end(),
                     [](const VisibleSprite& a, const VisibleSprite& b) {
                         return a.camera.depth > b.camera.depth;
                     });
    for (const VisibleSprite& item : visible) {
        const Texture& texture = textures(item.sprite->textureId);
        if (texture.width <= 0 || texture.height <= 0) {
            throw std::invalid_argument("Sprite texture has invalid dimensions");
        }
        const double spriteHeight = projectedSize(item.camera.depth, focal) * item.sprite->scale;
        const double spriteWidth = spriteHeight * texture.width / texture.height;
        const double centerX = screenXForCameraX(item.camera.cameraX, frame.width());
        if (!std::isfinite(spriteWidth) || !std::isfinite(spriteHeight) ||
            !std::isfinite(centerX) || spriteWidth <= 0.0 || spriteHeight <= 0.0) {
            continue;
        }
        const double left = centerX - spriteWidth / 2.0;
        const double right = centerX + spriteWidth / 2.0;
        const double top = 0.5 * frame.height() - spriteHeight / 2.0;
        const double bottom = 0.5 * frame.height() + spriteHeight / 2.0;
        const int xStart = clipStart(left, frame.width());
        const int xEnd = clipStart(right, frame.width());
        const int yStart = clipStart(top, frame.height());
        const int yEnd = clipStart(bottom, frame.height());
        for (int x = xStart; x < xEnd; ++x) {
            if (!(item.camera.depth < depthBuffer[static_cast<std::size_t>(x)])) {
                continue;
            }
            const int sourceX = texel(x, left, spriteWidth, texture.width);
            for (int y = yStart; y < yEnd; ++y) {
                const int sourceY = texel(y, top, spriteHeight, texture.height);
                const Color source = texture.sample(sourceX, sourceY);
                if ((source & 0xffu) == 0u) { continue; }
                const Color fogged = applyFog(source, item.camera.depth, fog);
                frame.setPixel(x, y, over(fogged, frame.pixelAt(x, y)));
            }
        }
    }
}

} // namespace wolf
