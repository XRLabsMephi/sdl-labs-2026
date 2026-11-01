#ifndef WORLD_PLAYER_H_
#define WORLD_PLAYER_H_

#include "math/vector.h"

struct Player {
  static constexpr float kDefaultFovScale = 0.66f;

  Vec2f position{0.0f, 0.0f};
  Vec2f direction{1.0f, 0.0f};
  Vec2f camera_plane{0.0f, kDefaultFovScale};
  float fov_scale = kDefaultFovScale;

  Player() = default;

  Player(const Vec2f& pos, const Vec2f& dir, float fov = kDefaultFovScale)
      : position(pos), fov_scale(fov) {
    SetDirection(dir, fov);
  }

  void SetDirection(const Vec2f& dir, float fov = kDefaultFovScale) {
    direction = dir.Normalized();
    fov_scale = fov;
    camera_plane = direction.Perpendicular() * fov_scale;
  }

  void Rotate(float angle_radians) {
    direction = direction.Rotated(angle_radians).Normalized();
    camera_plane = direction.Perpendicular() * fov_scale;
  }
};

#endif  // WORLD_PLAYER_H_
