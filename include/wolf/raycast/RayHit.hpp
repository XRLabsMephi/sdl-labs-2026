#pragma once

enum class HitSide {
    X,
    Y
};

struct RayHit {
    bool hit = false;
    double depth = 0.0;
    int mapX = 0;
    int mapY = 0;
    int wallId = 0;
    HitSide side = HitSide::X;
};