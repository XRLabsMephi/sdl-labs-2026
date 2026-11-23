#include "wolf/render/SpriteRenderer.hpp"

#include "wolf/render/Fog.hpp"
#include "wolf/render/FrameBuffer.hpp"
#include "wolf/texture/TextureStore.hpp"
#include "wolf/world/Player.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <vector>

namespace wolf
{

void SpriteRenderer::render(std::span<const Sprite> sprites, const Player& player,
                            const TextureStore& textures, std::span<const double> wallDepths,
                            const Fog& fog, FrameBuffer& frame) const
{
    if (wallDepths.size() != static_cast<std::size_t>(frame.width()))
    {
        throw std::invalid_argument("Expected one wall depth per screen column");
    }

    const auto forward = player.direction.normalized();
    const double planeWidth = player.cameraPlane.length();
    if (planeWidth <= 0.0)
    {
        return;
    }
    const auto right = player.cameraPlane * (1.0 / planeWidth);

    struct VisibleSprite
    {
        const Sprite* sprite;
        double depth;
        double lateral;
    };
    std::vector<VisibleSprite> visible;
    visible.reserve(sprites.size());
    for (const auto& sprite : sprites)
    {
        const auto relative = sprite.position - player.position;
        const double depth = relative.x * forward.x + relative.y * forward.y;
        if (depth > 1e-6 && sprite.scale > 0.0)
        {
            visible.push_back({&sprite, depth, relative.x * right.x + relative.y * right.y});
        }
    }
    std::stable_sort(visible.begin(), visible.end(), [](const auto& a, const auto& b)
    {
        return a.depth > b.depth;
    });

    const auto edge = [](double coordinate, int limit)
    {
        return static_cast<int>(std::clamp(std::ceil(coordinate), 0.0, double(limit)));
    };
    for (const auto& item : visible)
    {
        const auto& texture = textures.get(item.sprite->textureId);
        const double height = frame.height() * item.sprite->scale / item.depth;
        const double width = frame.width() * item.sprite->scale * texture.width
            / (2.0 * planeWidth * item.depth * texture.height);
        const double centerX = frame.width() * (1.0 + item.lateral / (item.depth * planeWidth)) / 2.0;
        const double left = centerX - width / 2.0;
        const double top = (frame.height() - height) / 2.0;
        const int endX = edge(left + width, frame.width());
        const int startY = edge(top, frame.height());
        const int endY = edge(top + height, frame.height());

        for (int x = edge(left, frame.width()); x < endX; ++x)
        {
            if (item.depth >= wallDepths[x])
            {
                continue;
            }
            const int texX = std::min(texture.width - 1, static_cast<int>((x - left) / width * texture.width));
            for (int y = startY; y < endY; ++y)
            {
                const int texY = std::min(texture.height - 1, static_cast<int>((y - top) / height * texture.height));
                const auto pixel = texture.sample(texX, texY);
                if ((pixel >> 24U) != 0)
                {
                    frame.setPixel(x, y, fog.apply(pixel, item.depth));
                }
            }
        }
    }
}

} // namespace wolf
