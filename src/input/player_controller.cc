#include "input/player_controller.h"

#include <algorithm>
#include <cmath>

PlayerController::PlayerController() : config_{} {}

PlayerController::PlayerController(const Config& config) : config_(config) {}

bool PlayerController::CanMoveTo(const Map& map, const Vec2f& position,
                                 float radius) const {
  const float pos_x = position.x();
  const float pos_y = position.y();

  if (!std::isfinite(pos_x) || !std::isfinite(pos_y) || !std::isfinite(radius) ||
      radius < 0.0f) {
    return false;
  }

  if (radius == 0.0f) {
    const int cell_x = static_cast<int>(std::floor(pos_x));
    const int cell_y = static_cast<int>(std::floor(pos_y));
    return !map.IsWall(cell_x, cell_y);
  }

  const int min_x = static_cast<int>(std::floor(pos_x - radius));
  const int max_x = static_cast<int>(std::floor(pos_x + radius));
  const int min_y = static_cast<int>(std::floor(pos_y - radius));
  const int max_y = static_cast<int>(std::floor(pos_y + radius));

  const float radius_sq = radius * radius;

  for (int cy = min_y; cy <= max_y; ++cy) {
    for (int cx = min_x; cx <= max_x; ++cx) {
      if (map.IsWall(cx, cy)) {
        // Nearest point on tile [cx, cx + 1] x [cy, cy + 1] to position
        const float closest_x =
            std::clamp(pos_x, static_cast<float>(cx), static_cast<float>(cx + 1));
        const float closest_y =
            std::clamp(pos_y, static_cast<float>(cy), static_cast<float>(cy + 1));

        const float dx = pos_x - closest_x;
        const float dy = pos_y - closest_y;
        if (dx * dx + dy * dy < radius_sq) {
          return false;
        }
      }
    }
  }

  return true;
}

void PlayerController::Update(Player& player, const Map& map,
                              const InputState& input, float dt) const {
  if (dt <= 0.0f || !std::isfinite(dt)) {
    return;
  }

  // 1. Rotation: positive angle turns right (clockwise in screen coordinates)
  float rotation_input = 0.0f;
  if (input.rotate_left) {
    rotation_input -= 1.0f;
  }
  if (input.rotate_right) {
    rotation_input += 1.0f;
  }

  if (rotation_input != 0.0f) {
    const float angle_radians = rotation_input * config_.rotate_speed * dt;
    player.Rotate(angle_radians);
  }

  // 2. Translation
  Vec2f move_dir{0.0f, 0.0f};
  if (input.move_forward) {
    move_dir += player.direction;
  }
  if (input.move_backward) {
    move_dir -= player.direction;
  }

  const Vec2f strafe_dir = player.camera_plane.Normalized();
  if (input.strafe_right) {
    move_dir += strafe_dir;
  }
  if (input.strafe_left) {
    move_dir -= strafe_dir;
  }

  if (move_dir.LengthSquared() == 0.0f) {
    return;
  }

  move_dir.Normalize();
  const Vec2f move_offset = move_dir * (config_.move_speed * dt);
  const Vec2f target_pos = player.position + move_offset;

  // 3. Collision and axis-independent sliding
  if (CanMoveTo(map, target_pos, config_.collision_radius)) {
    player.position = target_pos;
    return;
  }

  const Vec2f try_x{target_pos.x(), player.position.y()};
  const Vec2f try_y{player.position.x(), target_pos.y()};

  const bool can_move_x = CanMoveTo(map, try_x, config_.collision_radius);
  const bool can_move_y = CanMoveTo(map, try_y, config_.collision_radius);

  if (can_move_x && !can_move_y) {
    player.position = try_x;
  } else if (can_move_y && !can_move_x) {
    player.position = try_y;
  } else if (can_move_x && can_move_y) {
    // Both axes free individually, but diagonal corner blocked.
    // Slide along the axis with the larger displacement.
    if (std::abs(move_offset.x()) >= std::abs(move_offset.y())) {
      player.position = try_x;
    } else {
      player.position = try_y;
    }
  }
}
