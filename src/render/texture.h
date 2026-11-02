#ifndef SOFTWAREGRAPHICSLABS_RENDER_TEXTURE_H_
#define SOFTWAREGRAPHICSLABS_RENDER_TEXTURE_H_

#include <algorithm>
#include <cstdint>
#include <vector>

#include "core/color.h"

struct Texture {
  int width = 0;
  int height = 0;
  std::vector<uint32_t> pixels;

  Texture() = default;

  Texture(int w, int h, std::vector<uint32_t> pix)
      : width(w), height(h), pixels(std::move(pix)) {}

  Texture(int w, int h, Color fill_color)
      : width(w),
        height(h),
        pixels(static_cast<size_t>(std::max(0, w * h)), fill_color.GetValue()) {}

  [[nodiscard]] bool IsValid() const noexcept {
    return width > 0 && height > 0 &&
           static_cast<size_t>(width * height) == pixels.size();
  }

  // Соответствует контракту методички: uint32_t sample(int x, int y) const
  [[nodiscard]] uint32_t sample(int x, int y) const noexcept {
    if (!IsValid()) {
      return 0;
    }
    x = std::clamp(x, 0, width - 1);
    y = std::clamp(y, 0, height - 1);
    return pixels[static_cast<size_t>(y * width + x)];
  }

  [[nodiscard]] Color Sample(int x, int y) const noexcept {
    if (!IsValid()) {
      return Color::kBlack;
    }
    const uint32_t val = sample(x, y);
    return Color(static_cast<uint8_t>((val >> 24) & 0xFF),
                 static_cast<uint8_t>((val >> 16) & 0xFF),
                 static_cast<uint8_t>((val >> 8) & 0xFF),
                 static_cast<uint8_t>(val & 0xFF));
  }

  [[nodiscard]] Color SampleUV(double u, double v) const noexcept {
    if (!IsValid()) {
      return Color::kBlack;
    }
    u = std::clamp(u, 0.0, 0.999999);
    v = std::clamp(v, 0.0, 0.999999);
    const int x = static_cast<int>(u * width);
    const int y = static_cast<int>(v * height);
    return Sample(x, y);
  }
};

inline bool operator==(const Texture& lhs, const Texture& rhs) noexcept {
  return lhs.width == rhs.width && lhs.height == rhs.height &&
         lhs.pixels == rhs.pixels;
}

#endif  // SOFTWAREGRAPHICSLABS_RENDER_TEXTURE_H_
