#pragma once

namespace raycast {
    // ray-wall intersection.
    struct RayHit {
        bool           hit = false; // hit with a wall
        double       depth = 0.0;   // distance to screen
        int mapX = 0, mapY = 0;     // (x, y) cell, hit with ray
        int         wallId = 0;     // cell content (color / texture)
        int           side = 0;     // 0 = X-side (vertical), 1 = Y-side (horizontal)
    };
}