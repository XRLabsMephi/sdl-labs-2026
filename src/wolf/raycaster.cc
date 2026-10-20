#include "wolf/raycaster.h"

#include <cmath>
#include <limits>

#include "world/map.h"

RayHit Raycaster::CastRay(const Map& map, const Vec2f& position,
                          const Vec2f& ray_direction) const {
  const double pos_x = position.x();
  const double pos_y = position.y();
  const double dir_x = ray_direction.x();
  const double dir_y = ray_direction.y();

  // Проверяем диапазон до преобразования вещественных координат в целые.
  if (!std::isfinite(pos_x) || !std::isfinite(pos_y) || !std::isfinite(dir_x) ||
      !std::isfinite(dir_y) || (dir_x == 0.0 && dir_y == 0.0) || pos_x < 0.0 ||
      pos_y < 0.0 || pos_x >= map.GetWidth() || pos_y >= map.GetHeight()) {
    return {};
  }

  int cell_x = static_cast<int>(std::floor(pos_x));
  int cell_y = static_cast<int>(std::floor(pos_y));
  if (map.IsWall(cell_x, cell_y)) {
    return {};
  }

  const int step_x = dir_x < 0.0 ? -1 : 1;
  const int step_y = dir_y < 0.0 ? -1 : 1;
  const double infinity = std::numeric_limits<double>::infinity();

  // Расстояния здесь выражены в параметре t луча, а не в евклидовой длине.
  const double delta_dist_x = dir_x == 0.0 ? infinity : std::abs(1.0 / dir_x);
  const double delta_dist_y = dir_y == 0.0 ? infinity : std::abs(1.0 / dir_y);
  double side_dist_x = infinity;
  double side_dist_y = infinity;

  // При нулевой компоненте луч не пересекает границы по этой координате.
  // Обрабатываем её явно, чтобы на линии сетки не получить 0 * infinity.
  if (dir_x < 0.0) {
    side_dist_x = (pos_x - cell_x) * delta_dist_x;
  } else if (dir_x > 0.0) {
    side_dist_x = (cell_x + 1.0 - pos_x) * delta_dist_x;
  }
  if (dir_y < 0.0) {
    side_dist_y = (pos_y - cell_y) * delta_dist_y;
  } else if (dir_y > 0.0) {
    side_dist_y = (cell_y + 1.0 - pos_y) * delta_dist_y;
  }

  while (true) {
    double depth;
    WallSide side;
    if (side_dist_x < side_dist_y) {
      depth = side_dist_x;
      side_dist_x += delta_dist_x;
      cell_x += step_x;
      side = WallSide::kX;
    } else {
      depth = side_dist_y;
      side_dist_y += delta_dist_y;
      cell_y += step_y;
      side = WallSide::kY;
    }

    // IsWall() считает область за картой стеной, но данных клеток там нет.
    if (!map.InBounds(cell_x, cell_y)) {
      return {};
    }
    if (map.IsWall(cell_x, cell_y)) {
      RayHit result;
      result.hit = true;
      result.depth = depth;
      result.cell_x = cell_x;
      result.cell_y = cell_y;
      result.wall_id = map.GetTile(cell_x, cell_y).type_id;
      result.side = side;
      return result;
    }
  }
}
