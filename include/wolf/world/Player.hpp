#pragma once
#include "wolf/core/Vec2.hpp"

namespace wolf::world {

    struct Player {
        //центр 1,1
        core::Vec2 position{1.5, 1.5};
        //взгляд
        core::Vec2 direction{1.0, 0.0};
        //fov 66
        core::Vec2 cameraPlane{0.0, 0.66};
    };

}