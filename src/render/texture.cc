#include "render/texture.h"

#include <algorithm>
#include <cmath>
#include <utility>

Texture::Texture(int width, int height, std::vector<uint32_t> pixels) {
  if (width <= 0 || height <= 0) {
    return;
  }
  const std::size_t expected =
      static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
  if (pixels.size() != expected) {
    return;
  }
  width_ = width;
  height_ = height;
  pixels_ = std::move(pixels);
}

Texture Texture::FromColor(int width, int height, Color color) {
  if (width <= 0 || height <= 0) {
    return {};
  }
  const std::size_t count =
      static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
  return Texture(width, height, std::vector<uint32_t>(count, color.GetValue()));
}

Color Texture::SampleUV(double u, double v) const {
  if (IsEmpty() || !std::isfinite(u) || !std::isfinite(v)) {
    return Color::kBlack;
  }
  const int x = std::min(static_cast<int>(std::clamp(u, 0.0, 1.0) * width_),
                         width_ - 1);
  const int y = std::min(static_cast<int>(std::clamp(v, 0.0, 1.0) * height_),
                         height_ - 1);
  return Sample(x, y);
}

int Texture::Wrap(int value, int modulo) const {
  int wrapped = value % modulo;
  if (wrapped < 0) {
    wrapped += modulo;
  }
  return wrapped;
}

int Texture::PixelIndex(int x, int y) const {
  return Wrap(y, height_) * width_ + Wrap(x, width_);
}

uint32_t Texture::SampleRaw(int x, int y) const {
  if (pixels_.empty()) {
    return Color::kBlack.GetValue();
  }
  return pixels_[PixelIndex(x, y)];
}

Color Texture::Sample(int x, int y) const { return Color(SampleRaw(x, y)); }
