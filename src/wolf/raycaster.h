#ifndef WOLF_RAYCASTER_H_
#define WOLF_RAYCASTER_H_

#include "math/vector.h"
#include "wolf/ray_hit.h"

class Map;

class Raycaster {
 public:
  // Пускает один луч по сетке с единичными клетками: x растёт вправо, y вниз.
  // Начальная позиция должна быть внутри свободной клетки.
  // Компоненты направления должны быть конечными, а вектор ненулевым.
  // При неверных данных или выходе за карту возвращает RayHit без попадания.
  // Попадание в начальной точке может иметь нулевую глубину.
  // Если обе границы равноудалены, первым выполняется шаг по y.
  [[nodiscard]] RayHit CastRay(const Map& map, const Vec2f& position,
                               const Vec2f& ray_direction) const;
};

#endif  // WOLF_RAYCASTER_H_
