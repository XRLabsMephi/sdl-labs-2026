#pragma once

#include "wolf/core/Vec2.hpp"

namespace wolf {

struct Player {
    Vec2 position{2.5, 2.5};
    Vec2 direction{1.0, 0.0};
    Vec2 cameraPlane{0.0, 0.66};
};

}
