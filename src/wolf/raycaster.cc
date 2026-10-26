#include "wolf/raycaster.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "world/map.h"

namespace {

bool IsValidCamera(const Vec2f& direction, const Vec2f& camera_plane) {
  const double dir_x = direction.x();
  const double dir_y = direction.y();
  const double plane_x = camera_plane.x();
  const double plane_y = camera_plane.y();
  if (!std::isfinite(dir_x) || !std::isfinite(dir_y) ||
      !std::isfinite(plane_x) || !std::isfinite(plane_y)) {
    return false;
  }

  const double direction_length = std::hypot(dir_x, dir_y);
  const double plane_length = std::hypot(plane_x, plane_y);
  // Допуск учитывает погрешность float при поворотах векторов камеры.
  constexpr double kCameraTolerance = 1e-4;
  return std::abs(direction_length - 1.0) <= kCameraTolerance &&
         plane_length > 0.0 &&
         std::abs(dir_x * plane_x + dir_y * plane_y) <=
             kCameraTolerance * plane_length &&
         dir_x * plane_y - dir_y * plane_x > 0.0;
}

}  // namespace

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

WallColumn Raycaster::ProjectWall(const RayHit& ray_hit,
                                  int screen_height) const {
  WallColumn column;
  column.ray_hit = ray_hit;
  if (!ray_hit.hit || screen_height <= 0 || !std::isfinite(ray_hit.depth) ||
      ray_hit.depth < 0.0) {
    return column;
  }

  // Защита деления нужна только для проекции, глубину попадания не меняем.
  constexpr double kMinProjectionDepth = 1e-6;
  const double projection_depth = std::max(ray_hit.depth, kMinProjectionDepth);
  column.projected_height = screen_height / projection_depth;

  const double horizon = screen_height / 2.0;
  const double top = horizon - column.projected_height / 2.0;
  const double bottom = horizon + column.projected_height / 2.0;
  // Сначала обрезаем вещественные границы: близкая стена может не влезть в int.
  column.draw_start = static_cast<int>(
      std::floor(std::clamp(top, 0.0, static_cast<double>(screen_height))));
  column.draw_end = static_cast<int>(
      std::ceil(std::clamp(bottom, 0.0, static_cast<double>(screen_height))));
  return column;
}

std::vector<WallColumn> Raycaster::CastColumns(
    const Map& map, const Vec2f& position, const Vec2f& direction,
    const Vec2f& camera_plane, int screen_width, int screen_height) const {
  if (screen_width <= 0 || screen_height <= 0 ||
      !IsValidCamera(direction, camera_plane)) {
    return {};
  }

  std::vector<WallColumn> columns;
  columns.reserve(screen_width);
  for (int x = 0; x < screen_width; ++x) {
    const double camera_x = 2.0 * x / screen_width - 1.0;
    const Vec2f ray_direction = direction + camera_plane * camera_x;
    const RayHit ray_hit = CastRay(map, position, ray_direction);
    columns.push_back(ProjectWall(ray_hit, screen_height));
  }
  return columns;
}
