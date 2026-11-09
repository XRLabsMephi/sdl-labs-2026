#pragma once
#include "wolf/core/Vec2.hpp"
#include <cstdint>

namespace wolf {

enum class WallSide {
    Vertical = 0,   // Пересечение с вертикальной границей клетки (ось X)
    Horizontal = 1  // Пересечение с горизонтальной границей клетки (ось Y)
};

struct RayHit {
    bool hit = false;
    double depth = 0.0;
    int mapX = 0;
    int mapY = 0;
    int wallId = 0;
    WallSide side = WallSide::Vertical;
    double wallU = 0.0; // Позиция вдоль стены [0, 1)
    Vec2 hitPoint{0.0, 0.0};
};

} // namespace wolf
