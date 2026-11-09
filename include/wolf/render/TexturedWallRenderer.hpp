#pragma once

#include "wolf/effects/Fog.hpp"
#include "wolf/raycast/Raycaster.hpp"
#include "wolf/render/TextureLookup.hpp"

#include <span>

namespace wolf {

    class FrameBuffer;

    class TexturedWallRenderer {
    public:
        static void drawFrame(FrameBuffer& frame,
                              std::span<const WallSlice> columns,
                              const TextureLookup& textures,
                              const FogSettings& fog = {});
    };

}
