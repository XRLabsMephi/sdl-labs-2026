#include "wolf/sprite/SpriteRenderer.hpp"
#include "wolf/sprite/Sprite.hpp"
#include "wolf/world/Player.hpp"
#include "wolf/render/FrameBuffer.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace {
    bool validTexture(const Texture* texture) {
        if (!texture || texture->width <= 0 || texture->height <= 0) return false;
        std::size_t width = static_cast<std::size_t>(texture->width);
        return texture->pixels.size() / width == static_cast<std::size_t>(texture->height) && texture->pixels.size() % width == 0;
    }
    struct ProjectedSprite {
        const Texture* texture;
        double depth;
        double left;
        double top;
        double width;
        double height;
    };

    bool fartherFirst(const ProjectedSprite& first, const ProjectedSprite& second) {
        return first.depth > second.depth;
    }

    Color blend(Color source, Color background) {
        unsigned alpha = (source >> 24) & 255;
        unsigned inverse = 255 - alpha;
        unsigned red = (((source >> 16) & 255) * alpha + ((background >> 16) & 255) * inverse + 127) / 255;
        unsigned green = (((source >> 8) & 255) * alpha + ((background >> 8) & 255) * inverse + 127) / 255;
        unsigned blue = ((source & 255) * alpha + (background & 255) * inverse + 127) / 255;

        return makeColor(red, green, blue);
    }
}

void SpriteRenderer::render(const std::vector<Sprite>& sprites, const Player& player, FrameBuffer& frame, const TextureStore& textures, const std::vector<double>& depthBuffer) const {
    if (depthBuffer.size() != static_cast<std::size_t>(frame.width())) throw std::invalid_argument("Depth buffer width does not match framebuffer");

    double determinant = player.cameraPlane.x * player.direction.y - player.direction.x * player.cameraPlane.y;
    if (!std::isfinite(determinant) || std::abs(determinant) < 0.000000000001) return;

    double inverseDeterminant = 1.0 / determinant;
    std::vector<ProjectedSprite> projected;
    projected.reserve(sprites.size());

    for (const Sprite& sprite : sprites) {
        const Texture* texture = textures.get(sprite.textureId);
        if (!validTexture(texture) || !std::isfinite(sprite.scale) || sprite.scale <= 0.0) continue;

        Vec2 relative = sprite.position - player.position;

        double cameraX = inverseDeterminant * (player.direction.y * relative.x - player.direction.x * relative.y);
        double depth = inverseDeterminant * (-player.cameraPlane.y * relative.x + player.cameraPlane.x * relative.y);
        if (!std::isfinite(cameraX) || !std::isfinite(depth) || depth <= 0.000001) continue;

        double height = frame.height() * sprite.scale / depth;
        double width = height * texture->width / texture->height;
        double centerX = frame.width() * 0.5 * (1.0 + cameraX / depth);

        double left = centerX - width * 0.5;
        double bottom = frame.height() * 0.5 + frame.height() * 0.5 / depth;
        double top = bottom - height;

        if (!std::isfinite(width) || !std::isfinite(height) || !std::isfinite(left) || !std::isfinite(top)) continue;
        if (width <= 0.0 || height <= 0.0) continue;
        if (left >= frame.width() || left + width <= 0.0 || top >= frame.height() || top + height <= 0.0) continue;

        projected.push_back({texture, depth, left, top, width, height});
    }
    std::sort(projected.begin(), projected.end(), fartherFirst);
    for (const ProjectedSprite& sprite : projected) {
        int startX = static_cast<int>(std::floor(std::max(0.0, sprite.left)));
        int endX = static_cast<int>(std::ceil(std::min(static_cast<double>(frame.width()), sprite.left + sprite.width)));
        int startY = static_cast<int>(std::floor(std::max(0.0, sprite.top)));
        int endY = static_cast<int>(std::ceil(std::min(static_cast<double>(frame.height()), sprite.top + sprite.height)));

        for (int x = startX; x < endX; ++x) {
            if (sprite.depth >= depthBuffer[x]) continue;

            double u = std::clamp((x + 0.5 - sprite.left) / sprite.width, 0.0, 1.0);
            int textureX = std::clamp(static_cast<int>(u * sprite.texture->width), 0, sprite.texture->width - 1);

            for (int y = startY; y < endY; ++y) {
                double v = std::clamp((y + 0.5 - sprite.top) / sprite.height, 0.0, 1.0);
                int textureY = std::clamp(static_cast<int>(v * sprite.texture->height), 0, sprite.texture->height - 1);

                Color color = sprite.texture->pixels[static_cast<std::size_t>(textureY) * sprite.texture->width + textureX];
                unsigned alpha = (color >> 24) & 255;

                if (alpha == 0) continue;
                if (alpha < 255) color = blend(color, frame.data()[static_cast<std::size_t>(y) * frame.width() + x]);

                frame.setPixel(x, y, color);
            }
        }
    }
}