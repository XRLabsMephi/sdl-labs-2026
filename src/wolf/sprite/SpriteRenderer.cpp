#include "wolf/sprite/SpriteRenderer.hpp"

#include "wolf/core/Color.hpp"
#include "wolf/effects/Fog.hpp"
#include "wolf/render/FrameBuffer.hpp"
#include "wolf/texture/TextureStore.hpp"
#include "wolf/world/Player.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>
#include <vector>

namespace wolf {

void SpriteRenderer::render(std::span<const Sprite> sprites, const Player& player,
                             const TextureStore& textures, std::span<const double> wallDepth,
                             FrameBuffer& frame, const Fog* fog) const {
    if (wallDepth.size() != static_cast<std::size_t>(frame.width())) {
        throw std::invalid_argument("Размер буфера глубины не совпадает с шириной кадра");
    }
    const double determinant = player.cameraPlane.x * player.direction.y
        - player.direction.x * player.cameraPlane.y;
    if (!std::isfinite(determinant) || std::abs(determinant) < 1e-12) return;
    const double inverse = 1.0 / determinant;
    std::vector<std::pair<double, const Sprite*>> ordered;
    ordered.reserve(sprites.size());
    for (const auto& sprite : sprites) {
        const Vec2 relative = sprite.position - player.position;
        const double depth = inverse * (-player.cameraPlane.y * relative.x
                                         + player.cameraPlane.x * relative.y);
        if (std::isfinite(depth) && depth > 1e-4
            && std::isfinite(sprite.scale) && sprite.scale > 0.0) {
            ordered.emplace_back(depth, &sprite);
        }
    }
    std::stable_sort(ordered.begin(), ordered.end(), [](const auto& a, const auto& b) {
        return a.first > b.first;
    });

    for (const auto& [depth, sprite] : ordered) {
        const Vec2 relative = sprite->position - player.position;
        const double cameraX = inverse * (player.direction.y * relative.x
                                           - player.direction.x * relative.y);
        const Texture& texture = textures.get(sprite->textureId);
        const double centerX = frame.width() * 0.5 * (1.0 + cameraX / depth);
        const double height = frame.height() * sprite->scale / depth;
        const double width = height * texture.width / texture.height;
        const double bottom = frame.height() * 0.5 * (1.0 + 1.0 / depth);
        const double top = bottom - height;
        const double left = centerX - width * 0.5;
        const double right = centerX + width * 0.5;
        if (!std::isfinite(left) || !std::isfinite(right) || !std::isfinite(top)
            || !std::isfinite(bottom) || width <= 0.0 || height <= 0.0) continue;
        const int beginX = static_cast<int>(std::clamp(std::ceil(left), 0.0, double(frame.width())));
        const int endX = static_cast<int>(std::clamp(std::ceil(right), 0.0, double(frame.width())));
        const int beginY = static_cast<int>(std::clamp(std::ceil(top), 0.0, double(frame.height())));
        const int endY = static_cast<int>(std::clamp(std::ceil(bottom), 0.0, double(frame.height())));
        for (int x = beginX; x < endX; ++x) {
            if (depth >= wallDepth[x]) continue;
            const int textureX = static_cast<int>((x + 0.5 - left) * texture.width / width);
            for (int y = beginY; y < endY; ++y) {
                const int textureY = static_cast<int>((y + 0.5 - top) * texture.height / height);
                auto color = texture.sample(textureX, textureY);
                if ((color >> 24) == 0) continue;
                if (fog) color = fog->apply(color, depth);
                const auto background = frame.data()[static_cast<std::size_t>(y) * frame.width() + x];
                frame.setPixel(x, y, blend(color, background));
            }
        }
    }
}

}
