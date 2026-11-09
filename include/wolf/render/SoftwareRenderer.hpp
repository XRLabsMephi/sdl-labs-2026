#pragma once

#include "wolf/raycast/Raycaster.hpp"

#include <vector>

namespace wolf {

    class FrameBuffer;

    class SoftwareRenderer {
        public:
            static void drawFrame(FrameBuffer& frame, const std::vector<WallSlice>& columns) noexcept;
    };

}
