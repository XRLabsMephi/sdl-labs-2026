#include "wolf/sprite/SpriteRenderer.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <vector>

#include "wolf/texture/TextureStore.hpp"

namespace sprite {
    void SpriteRenderer::render(const world::Player& player,
                                std::span<const Sprite> sprites,
                                const texture::TextureStore& textures,
                                std::span<const double> wallDepth,
                                render::Framebuffer& framebuffer) const {
        const int width = framebuffer.width();
        const int height = framebuffer.height();
        if (wallDepth.size() != static_cast<std::size_t>(width)) {
            throw std::invalid_argument("Sprite wall depth buffer must match framebuffer width");
        }

        const auto direction = player.direction();
        const auto plane = player.cameraPlane();
        // Inverse of [plane direction]: use the same camera basis as the wall rays.
        const double determinant = plane.x * direction.y - direction.x * plane.y;
        if (!std::isfinite(determinant) || std::abs(determinant) < 1e-12) return;

        struct Projected {
            const texture::Texture* texture;
            double depth;
            double left, top, width, height;
        };
        std::vector<Projected> projected;
        projected.reserve(sprites.size());
        for (const auto& sprite : sprites) {
            const auto* texture = textures.get(sprite.textureId);
            if (!texture || !texture->valid() || !std::isfinite(sprite.scale) || sprite.scale <= 0.0) continue;

            const auto relative = sprite.position - player.position();
            const double cameraX = (direction.y * relative.x - direction.x * relative.y) / determinant;
            const double depth = (-plane.y * relative.x + plane.x * relative.y) / determinant;
            if (!std::isfinite(cameraX) || !std::isfinite(depth) || depth <= 1e-6) continue;

            const double spriteHeight = height * sprite.scale / depth;
            const double spriteWidth = spriteHeight * texture->width / texture->height;
            const double centerX = width * 0.5 * (1.0 + cameraX / depth);
            const double left = centerX - spriteWidth * 0.5;
            const double top = height * 0.5 + height * 0.5 / depth - spriteHeight;
            if (!std::isfinite(spriteHeight) || !std::isfinite(spriteWidth) ||
                !std::isfinite(left) || !std::isfinite(top) ||
                !std::isfinite(left + spriteWidth) || !std::isfinite(top + spriteHeight) ||
                spriteWidth <= 0.0 || spriteHeight <= 0.0) continue;
            if (left >= width || top >= height || left + spriteWidth <= 0.0 || top + spriteHeight <= 0.0) continue;
            projected.push_back({texture, depth, left, top, spriteWidth, spriteHeight});
        }

        // Camera depth, rather than radial distance, orders parallel billboards correctly.
        std::stable_sort(projected.begin(), projected.end(), [](const auto& a, const auto& b) {
            return a.depth > b.depth;
        });

        for (const auto& item : projected) {
            // Clip in floating point before converting to int, including near-plane sprites.
            const auto edge = [](double value, int limit) {
                return static_cast<int>(std::clamp(std::ceil(value - 0.5), 0.0, static_cast<double>(limit)));
            };
            const int startX = edge(item.left, width);
            const int endX = edge(item.left + item.width, width);
            const int startY = edge(item.top, height);
            const int endY = edge(item.top + item.height, height);
            for (int x = startX; x < endX; ++x) {
                if (!(item.depth < wallDepth[x])) continue;
                const int textureX = static_cast<int>(std::clamp(
                    (x + 0.5 - item.left) / item.width, 0.0, 1.0) * item.texture->width);
                for (int y = startY; y < endY; ++y) {
                    const int textureY = static_cast<int>(std::clamp(
                        (y + 0.5 - item.top) / item.height, 0.0, 1.0) * item.texture->height);
                    const auto color = item.texture->sample(textureX, textureY);
                    // Alpha cutout: zero is transparent; all other values are opaque.
                    if ((color >> 24U) == 0) continue;
                    // Per-pixel opaque depth also handles holes in cutout walls.
                    // Transparent walls stay deferred and can cover this sprite later.
                    framebuffer.drawFragment(x, y, {
                        static_cast<std::uint8_t>(color), static_cast<std::uint8_t>(color >> 8U),
                        static_cast<std::uint8_t>(color >> 16U)}, item.depth);
                }
            }
        }
    }
}
