#pragma once

#include <limits>

namespace wolf
{

enum class HitSide
{
    X, // The ray crossed a vertical grid line.
    Y  // The ray crossed a horizontal grid line.
};

struct RayHit
{
    bool hit = false;
    double depth = std::numeric_limits<double>::infinity();
    int cellX = -1;
    int cellY = -1;
    int wallId = 0;
    HitSide side = HitSide::X;
};

} // namespace wolf
