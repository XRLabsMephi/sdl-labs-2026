#pragma once

#include "wolf/effects/Fog.hpp"
#include "wolf/raycast/Raycaster.hpp"
#include "wolf/render/TextureLookup.hpp"

#include <span>

namespace wolf {

class FrameBuffer;

class TexturedWallRenderer {
public:
    // Draws ceiling, floor, and textured walls in a CPU FrameBuffer.
    // Pixel coordinates, wallU, and depth are supplied by Raycaster.
    static void drawFrame(FrameBuffer& frame, std::span<const WallSlice> columns,
                          const TextureLookup& textures,
                          const FogSettings& fog = {});
};

} // namespace wolf
