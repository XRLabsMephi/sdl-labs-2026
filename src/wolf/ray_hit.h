#ifndef WOLF_RAY_HIT_H_
#define WOLF_RAY_HIT_H_

#include <limits>

#include "world/tile.h"

// Ось последнего шага при входе в стену; знак направления здесь не хранится.
enum class WallSide {
  kNone,
  kX,  // Вертикальная граница клетки: x постоянен.
  kY,  // Горизонтальная граница клетки: y постоянен.
};

struct RayHit {
  bool hit = false;

  // Параметр t в формуле position + t * ray_direction.
  // Равен глубине вдоль направления камеры для лучей direction + plane * camera_x,
  // если direction единичный, а plane перпендикулярен ему.
  // Направления отдельных лучей не нормализуем.
  double depth = std::numeric_limits<double>::infinity();

  int cell_x = -1;
  int cell_y = -1;
  TileId wall_id = 0;
  WallSide side = WallSide::kNone;
};

#endif  // WOLF_RAY_HIT_H_
