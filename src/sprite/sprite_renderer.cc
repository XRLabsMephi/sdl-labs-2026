#include "sprite/sprite_renderer.h"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace {

constexpr double kMinDepth = 1e-6;

int WrapTexel(int value, int modulo) {
  int wrapped = value % modulo;
  if (wrapped < 0) {
    wrapped += modulo;
  }
  return wrapped;
}

}

bool SpriteRenderer::IsTransparent(Color color) {
  return color.GetA() == 0 || color == Color::kMagentaKey;
}

SpriteProjection SpriteRenderer::Project(const Sprite& sprite,
                                         const Player& player,
                                         const Texture& texture,
                                         int screen_width,
                                         int screen_height) const {
  SpriteProjection projection;
  if (texture.IsEmpty() || screen_width <= 0 || screen_height <= 0 ||
      !(sprite.scale > 0.0) || !std::isfinite(sprite.scale)) {
    return projection;
  }

  const double dir_x = player.direction.x();
  const double dir_y = player.direction.y();
  const double plane_x = player.camera_plane.x();
  const double plane_y = player.camera_plane.y();
  const double rel_x =
      static_cast<double>(sprite.position.x()) - player.position.x();
  const double rel_y =
      static_cast<double>(sprite.position.y()) - player.position.y();
  if (!std::isfinite(dir_x) || !std::isfinite(dir_y) || !std::isfinite(plane_x) ||
      !std::isfinite(plane_y) || !std::isfinite(rel_x) || !std::isfinite(rel_y)) {
    return projection;
  }

  // relative = depth * direction + u * camera_plane.
  const double det = dir_x * plane_y - dir_y * plane_x;
  if (!std::isfinite(det) || std::abs(det) < kMinDepth) {
    return projection;
  }
  const double inv_det = 1.0 / det;
  const double depth = inv_det * (rel_x * plane_y - plane_x * rel_y);
  const double u = inv_det * (dir_x * rel_y - rel_x * dir_y);
  if (!std::isfinite(depth) || !std::isfinite(u) || depth <= kMinDepth) {
    return projection;
  }

  const double camera_x = u / depth;
  if (!std::isfinite(camera_x)) {
    return projection;
  }

  const double aspect = static_cast<double>(texture.GetWidth()) /
                        static_cast<double>(texture.GetHeight());
  projection.depth = depth;
  projection.sprite_height = screen_height / depth * sprite.scale;
  projection.sprite_width = projection.sprite_height * aspect;
  projection.screen_x = (screen_width / 2.0) * (1.0 + camera_x);
  if (!std::isfinite(projection.sprite_width) ||
      !std::isfinite(projection.sprite_height) ||
      !std::isfinite(projection.screen_x) || projection.sprite_width <= 0.0 ||
      projection.sprite_height <= 0.0) {
    return projection;
  }

  const double left = projection.screen_x - projection.sprite_width / 2.0;
  const double right = projection.screen_x + projection.sprite_width / 2.0;
  const double horizon = screen_height / 2.0;
  const double top = horizon - projection.sprite_height / 2.0;
  const double bottom = horizon + projection.sprite_height / 2.0;

  projection.draw_start_x = static_cast<int>(
      std::floor(std::clamp(left, 0.0, static_cast<double>(screen_width))));
  projection.draw_end_x = static_cast<int>(
      std::ceil(std::clamp(right, 0.0, static_cast<double>(screen_width))));
  projection.draw_start_y = static_cast<int>(
      std::floor(std::clamp(top, 0.0, static_cast<double>(screen_height))));
  projection.draw_end_y = static_cast<int>(
      std::ceil(std::clamp(bottom, 0.0, static_cast<double>(screen_height))));
  projection.visible =
      projection.draw_start_x < projection.draw_end_x &&
      projection.draw_start_y < projection.draw_end_y;
  return projection;
}

void SpriteRenderer::DrawProjected(
    const SpriteProjection& projection, const Texture& texture,
    const std::vector<double>& depth_buffer, FrameBuffer& frame_buffer,
    const Fog& fog) const {
  const double left = projection.screen_x - projection.sprite_width / 2.0;
  const double horizon = frame_buffer.GetHeight() / 2.0;
  const double top = horizon - projection.sprite_height / 2.0;
  const int tex_width = texture.GetWidth();
  const int tex_height = texture.GetHeight();

  for (int x = projection.draw_start_x; x < projection.draw_end_x; ++x) {
    if (x < 0 || x >= static_cast<int>(depth_buffer.size())) {
      continue;
    }
    if (!(projection.depth < depth_buffer[x])) {
      continue;
    }

    const int tex_x = WrapTexel(
        static_cast<int>((x - left) / projection.sprite_width * tex_width),
        tex_width);

    for (int y = projection.draw_start_y; y < projection.draw_end_y; ++y) {
      const int tex_y = WrapTexel(
          static_cast<int>((y - top) / projection.sprite_height * tex_height),
          tex_height);
      const Color texel = texture.Sample(tex_x, tex_y);
      if (IsTransparent(texel)) {
        continue;
      }
      frame_buffer.SetPixel(x, y, fog.Apply(texel, projection.depth));
    }
  }
}

void SpriteRenderer::Draw(const std::vector<Sprite>& sprites,
                          const Player& player,
                          const std::vector<Texture>& textures,
                          const std::vector<double>& depth_buffer,
                          FrameBuffer& frame_buffer, const Fog& fog) const {
  struct Item {
    SpriteProjection projection;
    const Texture* texture = nullptr;
  };

  std::vector<Item> items;
  items.reserve(sprites.size());
  for (const Sprite& sprite : sprites) {
    if (sprite.texture_id < 0 ||
        sprite.texture_id >= static_cast<int>(textures.size())) {
      continue;
    }
    const Texture& texture = textures[sprite.texture_id];
    SpriteProjection projection =
        Project(sprite, player, texture, frame_buffer.GetWidth(),
                frame_buffer.GetHeight());
    if (!projection.visible) {
      continue;
    }
    items.push_back(Item{projection, &texture});
  }

  std::sort(items.begin(), items.end(), [](const Item& lhs, const Item& rhs) {
    return lhs.projection.depth > rhs.projection.depth;
  });

  for (const Item& item : items) {
    DrawProjected(item.projection, *item.texture, depth_buffer, frame_buffer,
                  fog);
  }
}
