#ifndef RENDER_TEXTURE_H_
#define RENDER_TEXTURE_H_

#include <cstdint>
#include <vector>

#include "core/color.h"

// Программная текстура в том же RGBA8888, что и FrameBuffer.
// SDL_Surface здесь не хранится: загрузка файла — задача TextureStore.
class Texture {
 public:
  Texture() = default;
  Texture(int width, int height, std::vector<uint32_t> pixels);

  static Texture FromColor(int width, int height, Color color);

  [[nodiscard]] int GetWidth() const { return width_; }
  [[nodiscard]] int GetHeight() const { return height_; }
  [[nodiscard]] bool IsEmpty() const { return pixels_.empty(); }

  // Координаты берутся по модулю размера, в том числе отрицательные.
  [[nodiscard]] Color Sample(int x, int y) const;
  [[nodiscard]] uint32_t SampleRaw(int x, int y) const;

 private:
  [[nodiscard]] int Wrap(int value, int modulo) const;
  [[nodiscard]] int PixelIndex(int x, int y) const;

  int width_ = 0;
  int height_ = 0;
  std::vector<uint32_t> pixels_;
};

#endif  // RENDER_TEXTURE_H_
