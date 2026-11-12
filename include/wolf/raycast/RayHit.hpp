#pragma once

namespace wolf::raycast {
    struct RayHit {
        double wallDist = 0.0;
        int wallType = 0;
        bool hit = false;
        // 0 hit с x, 1 с y
        int side = 0;
    };
}