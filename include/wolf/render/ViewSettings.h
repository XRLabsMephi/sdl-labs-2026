#pragma once

#include <core/Color.h>

struct ViewSettings {
    bool fish_eye = false;
    bool angle_projection = false;
    core::Color wall_color{255, 175, 119};
    core::Color ground_color{76, 76, 76};
    core::Color sky_color{147, 89, 124};
    double minimal_brightness = 0.6;
    double darkening_distance = 30.0;
};
