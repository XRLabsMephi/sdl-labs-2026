#ifndef WOLF_RAYCASTER_H_
#define WOLF_RAYCASTER_H_

#include <vector>

#include "math/vector.h"
#include "wolf/ray_hit.h"

class Map;

// Данные одной вертикальной полосы стены для последующей отрисовки.
struct WallColumn {
  RayHit ray_hit;

  // Высота до обрезки экраном. Для проекции глубина ограничена снизу 1e-6,
  // но исходное значение ray_hit.depth остаётся неизменным.
  double projected_height = 0.0;

  // Строки [draw_start, draw_end): верх включён, низ не включён.
  // При пустом диапазоне рисовать стену не нужно.
  int draw_start = 0;
  int draw_end = 0;
};

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

  // Проекция стены единичной высоты с горизонтом посередине экрана.
  // Верх округляется вниз, низ вверх; диапазон ограничен экраном.
  // Промах, отрицательная/нечисловая/бесконечная глубина или высота экрана <= 0
  // дают пустую полосу. Переданный RayHit сохраняется без изменений.
  [[nodiscard]] WallColumn ProjectWall(const RayHit& ray_hit,
                                       int screen_height) const;

  // Строит один луч для каждого столбца: индекс результата равен экранному x.
  // direction должен быть единичным, camera_plane — перпендикулярным ему
  // и направленным вправо на экране. При взгляде вправо (1, 0) это (0, +k).
  // Используется camera_x = 2.0 * x / screen_width - 1.0.
  // Отдельные лучи не нормализуются: их параметр t уже задаёт глубину камеры.
  // Некорректная камера или размеры экрана дают пустой вектор.
  // Промахи, включая неверную позицию, сохраняют свои столбцы с пустой полосой.
  [[nodiscard]] std::vector<WallColumn> CastColumns(
      const Map& map, const Vec2f& position, const Vec2f& direction,
      const Vec2f& camera_plane, int screen_width, int screen_height) const;
};

#endif  // WOLF_RAYCASTER_H_
