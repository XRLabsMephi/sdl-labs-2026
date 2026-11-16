#ifndef EFFECTS_FOG_H_
#define EFFECTS_FOG_H_

#include <cmath>
#include <cstdint>

#include "core/color.h"

// Линейный туман по той же перпендикулярной глубине, что у стен и спрайтов.
class Fog {
 public:
  static constexpr double kDefaultStart = 4.0;
  static constexpr double kDefaultEnd = 12.0;

  Fog() = default;
  Fog(double start, double end, Color color)
      : start_(start), end_(end), color_(color) {}

  [[nodiscard]] double GetStart() const { return start_; }
  [[nodiscard]] double GetEnd() const { return end_; }
  [[nodiscard]] Color GetColor() const { return color_; }

  // 0 — объект без тумана, 1 — полностью цвет тумана.
  [[nodiscard]] double Factor(double depth) const {
    if (!std::isfinite(depth) || depth >= end_) {
      return 1.0;
    }
    if (depth <= start_) {
      return 0.0;
    }
    if (!(end_ > start_)) {
      return 1.0;
    }
    return (depth - start_) / (end_ - start_);
  }

  [[nodiscard]] Color Apply(Color scene_color, double depth) const {
    const double t = Factor(depth);
    if (t <= 0.0) {
      return scene_color;
    }
    if (t >= 1.0) {
      return color_;
    }
    const double one_minus_t = 1.0 - t;
    const auto mix = [t, one_minus_t](uint8_t scene, uint8_t fog) {
      return static_cast<uint8_t>(
          std::lround(scene * one_minus_t + fog * t));
    };
    return Color(mix(scene_color.GetR(), color_.GetR()),
                 mix(scene_color.GetG(), color_.GetG()),
                 mix(scene_color.GetB(), color_.GetB()),
                 scene_color.GetA());
  }

 private:
  double start_ = kDefaultStart;
  double end_ = kDefaultEnd;
  Color color_ = Color::kFog;
};

#endif  // EFFECTS_FOG_H_
