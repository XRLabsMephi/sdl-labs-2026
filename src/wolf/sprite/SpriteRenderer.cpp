#include "wolf/sprite/SpriteRenderer.hpp"
#include <cmath>
#include <algorithm>
#include <vector>

namespace wolf::sprite {
    void SpriteRenderer::render(render::FrameBuffer& fb,  const world::Player& player, const std::vector<Sprite>& sprites,
            const std::vector<double>& depthBuffer, const texture::TextureStore& textureStore) {
        int width = fb.getWidth();
        int height = fb.getHeight();
        std::vector<Sprite> sortedSprites = sprites;
        std::sort(sortedSprites.begin(), sortedSprites.end(),
            [&player](const Sprite& a, const Sprite& b) {
                double firstDist = (player.position.x - a.position.x) * (player.position.x - a.position.x) +
                    (player.position.y - a.position.y) * (player.position.y - a.position.y);
                double secDist = (player.position.x - b.position.x) * (player.position.x - b.position.x) +
                               (player.position.y - b.position.y) * (player.position.y - b.position.y);
                return firstDist > secDist;
            });
        for (const auto& sprite : sortedSprites) {

        }
    }
};
