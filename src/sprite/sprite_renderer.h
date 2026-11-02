#ifndef SPRITE_SPRITE_RENDERER_H_
#define SPRITE_SPRITE_RENDERER_H_

#include <vector>

#include "effects/fog.h"
#include "render/frame_buffer.h"
#include "render/texture.h"
#include "world/sprite.h"
#include "world/player.h"

// Экранная проекция billboard-спрайта. Диапазоны столбцов и строк полуоткрытые:
// [draw_start, draw_end). depth — удаление вперёд относительно направления камеры.
struct SpriteProjection {
  bool visible = false;
  double depth = 0.0;
  double sprite_width = 0.0;
  double sprite_height = 0.0;
  double screen_x = 0.0;
  int draw_start_x = 0;
  int draw_end_x = 0;
  int draw_start_y = 0;
  int draw_end_y = 0;
};

class SpriteRenderer {
 public:
  // Пиксель не пишется, если альфа равна 0 или цвет совпадает с ключом.
  static bool IsTransparent(Color color);

  // Переводит предмет в пространство камеры и считает экранный прямоугольник.
  // depth > 0 означает «перед камерой». При промахе visible = false.
  [[nodiscard]] SpriteProjection Project(const Sprite& sprite,
                                         const Player& player,
                                         const Texture& texture,
                                         int screen_width,
                                         int screen_height) const;

  // Сначала дальние предметы, затем ближние. Для каждого столбца спрайт
  // рисуется только если его depth меньше depth_buffer[x] ближайшей стены.
  // Размер depth_buffer должен быть равен ширине кадра.
  void Draw(const std::vector<Sprite>& sprites, const Player& player,
            const std::vector<Texture>& textures,
            const std::vector<double>& depth_buffer,
            FrameBuffer& frame_buffer, const Fog& fog) const;

 private:
  void DrawProjected(const SpriteProjection& projection,
                     const Texture& texture,
                     const std::vector<double>& depth_buffer,
                     FrameBuffer& frame_buffer, const Fog& fog) const;
};

#endif  // SPRITE_SPRITE_RENDERER_H_
