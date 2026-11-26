#include "wolf/sprite/SpriteRenderer.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "wolf/render/Fog.hpp"
#include "wolf/render/FrameBuffer.hpp"
#include "wolf/texture/TextureStore.hpp"
#include "wolf/world/Player.hpp"

namespace wolf {
namespace {

struct VisibleSprite {
    const Sprite* sprite;
    double depth;
    double centerX;
};

// Rasterize pixel centers within a half-open projected interval.
int clippedEdge(double edge, int limit) {
    return static_cast<int>(std::clamp(std::ceil(edge - 0.5), 0.0,
                                       static_cast<double>(limit)));
}

void drawSprite(FrameBuffer& frame, const VisibleSprite& visible,
                const Texture& texture, const std::vector<double>& walls, const Fog& fog) {
    const double height = (visible.sprite->scale / visible.depth) * frame.height();
    const double width = height * (static_cast<double>(texture.width) / texture.height);
    if (!(height > 0 && width > 0) || !std::isfinite(width) || !std::isfinite(height)) {
        return;
    }
    const double left = visible.centerX - width / 2;
    const double top = (frame.height() - height) / 2;
    const int xEnd = clippedEdge(visible.centerX + width / 2, frame.width());
    const int yEnd = clippedEdge((frame.height() + height) / 2, frame.height());
    const int yBegin = clippedEdge(top, frame.height());

    for (int x = clippedEdge(left, frame.width()); x < xEnd; ++x) {
        if (visible.depth >= walls[x]) {
            continue;
        }
        const int u = static_cast<int>(((x + 0.5 - left) / width) * texture.width);
        for (int y = yBegin; y < yEnd; ++y) {
            const int v = static_cast<int>(((y + 0.5 - top) / height) * texture.height);
            const Color pixel = texture.sample(u, v);
            if (pixel.a != 0) {
                frame.setPixel(x, y, fog.apply(pixel, visible.depth));
            }
        }
    }
}

}

void SpriteRenderer::render(FrameBuffer& frame, const std::vector<Sprite>& sprites,
                            const Player& player, const TextureStore& textures,
                            const std::vector<double>& depthBuffer, const Fog& fog) const {
    if (depthBuffer.size() != static_cast<std::size_t>(frame.width())) {
        throw std::invalid_argument("Sprite rendering needs one wall depth per column");
    }
    const Vec2 plane = player.cameraPlane;
    const double planeSquared = plane.x * plane.x + plane.y * plane.y;
    std::vector<VisibleSprite> visible;
    visible.reserve(sprites.size());
    for (const Sprite& sprite : sprites) {
        const Vec2 offset{sprite.position.x - player.position.x,
                          sprite.position.y - player.position.y};
        const double depth = offset.x * player.direction.x + offset.y * player.direction.y;
        if (depth <= 0.0001 || !std::isfinite(depth)) {
            continue;
        }
        const double lateral = (offset.x * plane.x + offset.y * plane.y) / planeSquared;
        const double centerX = (1.0 + lateral / depth) * (frame.width() / 2.0);
        if (std::isfinite(centerX)) {
            visible.push_back({&sprite, depth, centerX});
        }
    }
    std::stable_sort(visible.begin(), visible.end(), [](const auto& a, const auto& b) {
        return a.depth > b.depth;
    });
    for (const VisibleSprite& item : visible) {
        drawSprite(frame, item, textures.get(item.sprite->textureId), depthBuffer, fog);
    }
}

}
