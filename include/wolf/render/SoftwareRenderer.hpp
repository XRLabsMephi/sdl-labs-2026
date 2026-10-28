#pragma once

#include "wolf/raycast/Raycaster.hpp"

#include <vector>

namespace wolf {

class FrameBuffer;

// Converts already-projected Raycaster columns into CPU pixel data; no SDL calls.
class SoftwareRenderer {
public:
    static void drawFrame(FrameBuffer& frame, const std::vector<WallSlice>& columns) noexcept;
};

} // namespace wolf
