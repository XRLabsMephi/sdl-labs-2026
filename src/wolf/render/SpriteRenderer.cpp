#include <render/SpriteRenderer.h>
#include <render/Projection.h>
#include <render/TextureProjection.h>
#include <algorithm>
#include <debug/Logger.h>

void render::SpriteRenderer::renderSprites() const {
    struct VisibleSprite {
        const world::Sprite* sprite;
        double camera_x;
        double depth;
    };

    if (wallDepths.size() != static_cast<std::size_t>(frame_width_)) {
        throw logging::Error(logging::ErrorCode::SpriteDepthBuffer, "Sprite depth buffer width mismatch");
    }

    const Vector2 forward = player_.getDirection();
    const Vector2 right{forward.y, -forward.x};
    const Projection projection(frame_width_, frame_height_, player_.getAngle(),
                                player_.getEyeHeight(), view_settings_);
    std::vector<VisibleSprite> visible;
    for (const auto& sprite : map_.getSprites()) {
        const Vector2 relative = sprite.position - player_.getPosition();
        const double depth = relative.dot(forward);
        if (depth > eps) {
            visible.push_back({&sprite, relative.dot(right), depth});
        }
    }

    std::stable_sort(visible.begin(), visible.end(),
        [](const auto& lhs, const auto& rhs) { return lhs.depth > rhs.depth; });

    for (const auto& [sprite, camera_x, depth] : visible) {
        const auto& texture = textures_.getOrEmpty(sprite->textureId);
        if (texture.width <= 0 || texture.height <= 0) continue;

        const double width = sprite->scale * (static_cast<double>(texture.width) / texture.height);
        if (!(width > 0.0) || !std::isfinite(width)) continue;
        const double left = camera_x - width / 2.0;
        const double right_edge = camera_x + width / 2.0;
        const auto clip = [this](double x) {
            return static_cast<int>(std::ceil(std::clamp(x, 0.0, static_cast<double>(frame_width_))));
        };
        const int x0 = clip(projection.screenX(left / depth));
        const int x_end = clip(projection.screenX(right_edge / depth));

        for (int x = x0; x < x_end; ++x) {
            // The depth buffer always stores the forward depth, even in fisheye mode.
            if (depth >= wallDepths[x]) continue;

            const double slope = projection.slopeAt(x);
            const double render_depth = projection.renderDepth(depth, slope);
            const double u = std::clamp((slope * depth - left) / width, 0.0, 1.0);
            const int tex_x = std::min(texture.width - 1, static_cast<int>(u * texture.width));
            DrawCallParams params{texture.column(tex_x), 0, 0, render_depth, 255};
            int y0, y_end;
            if (projectTextureColumn(projection.screenY(sprite->scale, render_depth),
                                     projection.screenY(0.0, render_depth), frame_height_,
                                     texture.height, y0, y_end, params)) {
                frame_buffer_.vline_sprite(x, y0, y_end - 1, params);
            }
        }
    }
}
