#ifndef WORLD_SPRITE_H_
#define WORLD_SPRITE_H_

#include "math/vector.h"

// Предмет сцены (billboard sprite) в мировых координатах.
struct Sprite {
  Vec2f position{0.0f, 0.0f};
  int texture_id = 0;
  double scale = 1.0;

  Sprite() = default;

  Sprite(const Vec2f& pos, int tex_id, double s = 1.0)
      : position(pos), texture_id(tex_id), scale(s) {}

  Sprite(float x, float y, int tex_id, double s = 1.0)
      : position{x, y}, texture_id(tex_id), scale(s) {}

  bool operator==(const Sprite& other) const noexcept {
    return position == other.position && texture_id == other.texture_id &&
           scale == other.scale;
  }
};

#endif  // WORLD_SPRITE_H_
