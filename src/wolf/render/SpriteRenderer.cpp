#include <render/SpriteRenderer.h>
#include <algorithm>
using namespace render;

//void SpriteRenderer::drawSprites(Vector2 forward, double tanHalfFov) const

void SpriteRenderer::renderSprites() const {
        struct VisibleSprite {
            const world::Sprite* sprite;
            double cameraX;
            double depth;
        };
        const Vector2 forward = player_.getDirection();
        const Vector2 right{forward.y, -forward.x};
        const double halfWidth = (frame_width_ - 1) / 2.0;
        const double horizontalFocalLength = halfWidth / tan(player_.getAngle() / 2.0f);
        const double eyeHeight = player_.getEyeHeight();
        const double horizon = frame_height_ - eyeHeight;
        constexpr int TEXTURE_UNIT = 1 << 16;

        std::vector<VisibleSprite> visible;
        for (const auto& sprite : map_.getSprites()) {
            const Vector2 relative = sprite.position - player_.getPosition();
            const double depth = viewSettings_.fish_eye? relative.length(): relative.dot(forward);
            if (depth > eps) {
                visible.push_back({&sprite, relative.dot(right), depth});
            }
        }

        // Blend nearer sprites over farther sprites and the wall pass.
        std::stable_sort(visible.begin(), visible.end(),
            [](const VisibleSprite& lhs, const VisibleSprite& rhs) {
                return lhs.depth > rhs.depth;
            });

        for (const auto& [sprite, cameraX, depth] : visible)
        {
            const auto& texture = textures_.getOrEmpty(sprite->textureId);
            if (&texture == &render::Texture::empty){
                continue;
            }
            const double centerX = halfWidth + horizontalFocalLength * cameraX / depth;
            const double projectedHeight = horizontalFocalLength * (sprite->scale / depth);
            const double projectedWidth = horizontalFocalLength * (sprite->scale / depth)
                * (static_cast<double>(texture.width) / texture.height);

            // Align the base of the sprite with the base of a wall at the same depth.
            const double bottom = horizon + horizontalFocalLength * (eyeHeight / frame_height_) / depth;
            const double top = bottom - projectedHeight;
            const double leftEdge = centerX - projectedWidth / 2.0;
            const double rightEdge = centerX + projectedWidth / 2.0;
            if (!std::isfinite(projectedWidth) || !std::isfinite(projectedHeight) ||
                projectedWidth <= 0.0 || projectedHeight <= 0.0) {
                continue;
                }

            // Keep the original bounds for texture coordinates after clipping.
            const auto clip = [](double edge, int limit) {
                return static_cast<int>(std::ceil(std::clamp(edge, 0.0, static_cast<double>(limit))));
            };
            const int x0 = clip(leftEdge, frame_width_);
            const int xEnd = clip(rightEdge, frame_width_);
            const int y0 = clip(top, frame_height_);
            const int yEnd = clip(bottom, frame_height_);
            if (x0 >= xEnd || y0 >= yEnd) {
                continue;
            }

            const auto lastTexel = texture.height * TEXTURE_UNIT - 1;
            const double textureStep = static_cast<double>(texture.height) * TEXTURE_UNIT / projectedHeight;
            const auto texY = static_cast<int>
            (std::clamp((y0 - top) / projectedHeight, 0.0, 1.0) * lastTexel);
            const auto step = yEnd - y0 > 1
                ? std::min(static_cast<int>(textureStep), (lastTexel - texY) / (yEnd - y0 - 1))
                : 0;

            for (int x = x0; x < xEnd; ++x) {
                if (depth >= wallDepths[x]) {
                    continue;
                }
                const int texX = std::min(texture.width - 1,
                    static_cast<int>(std::clamp((x - leftEdge) / projectedWidth, 0.0, 1.0) * texture.width));
                frame_buffer_.vline_sprite(x, y0, yEnd - 1, {
                    texture.column(texX), texY, step, static_cast<float>(depth), 255
                });
            }
        }
}