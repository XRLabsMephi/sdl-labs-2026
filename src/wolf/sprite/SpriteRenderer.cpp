#include "wolf/sprite/SpriteRenderer.hpp"
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <vector>

namespace wolf::sprite {

    void SpriteRenderer::render(render::FrameBuffer& fb, const world::Player& player, const std::vector<Sprite>& sprites,
            const std::vector<double>& depthBuffer, const texture::TextureStore& textureStore, const effects::Fog& fog) {
        int width = fb.getWidth();
        int height = fb.getHeight();
        if (width <= 0 || height <= 0) {
            throw std::runtime_error("width and height must be greater than 0");
        }
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
            double spriteX = sprite.position.x - player.position.x;
            double spriteY = sprite.position.y - player.position.y;
            double invDet = 1.0 / (player.cameraPlane.x * player.direction.y - player.direction.x * player.cameraPlane.y);
            double transformX = invDet * (player.direction.y * spriteX - player.direction.x * spriteY);
            double transformY = invDet * (-player.cameraPlane.y * spriteX + player.cameraPlane.x * spriteY);
            if (transformY <= 0.1) {
                continue;
            }
            int spriteScreenX = static_cast<int>((width / 2.0) * (1 + transformX / transformY));

            int spriteHeight = static_cast<int>(std::abs(height / transformY) * sprite.scale);
            int spriteWidth = static_cast<int>(std::abs(height / transformY) * sprite.scale);
            if (spriteHeight <= 0 || spriteWidth <= 0) {
                throw std::runtime_error("sprite height and width must be greater than 0");
            }
            int vMoveScreen = static_cast<int>((height / transformY) * (1.0 - sprite.scale) / 2.0);
            int drawStartY = -spriteHeight / 2 + height / 2 + vMoveScreen;
            int drawEndY = spriteHeight / 2 + height / 2 + vMoveScreen;
            drawStartY = std::max(0, drawStartY);
            drawEndY = std::min(height, drawEndY);

            int drawStartX = -spriteWidth / 2 + spriteScreenX;
            int drawEndX = spriteWidth / 2 + spriteScreenX;
            drawStartX = std::max(0, drawStartX);
            drawEndX = std::min(width, drawEndX);

            const auto& textura = textureStore.get(sprite.textureId);
            if (!textura.valid()) {
                continue;
            }

            for (int row = drawStartX; row < drawEndX; ++row) {
                if (transformY < depthBuffer[row]) {
                    int textureX = static_cast<int>(256 * (row - (-spriteWidth / 2 + spriteScreenX))
                        * textura.width / spriteWidth) / 256;
                    for (int y = drawStartY; y < drawEndY; ++y) {
                        int d = (y - vMoveScreen) * 256 - height * 128 + spriteHeight * 128;
                        int textureY = ((d * textura.height) / spriteHeight) / 256;
                        std::uint32_t color = textura.sample(textureX, textureY);
                        if ((color >> 24) == 0) {
                            continue;
                        }
                        color = fog.apply(color, transformY);
                        fb.setPixel(row, y, color);
                    }
                }
            }
        }
    }
}