#pragma once

#include "wolf/effects/Fog.hpp"
#include "wolf/render/TextureLookup.hpp"
#include "wolf/sprite/Sprite.hpp"

#include <span>

namespace wolf {

    class FrameBuffer;
    struct Player;

    class SpriteRenderer {
    public:
        static void drawSprites(FrameBuffer& frame,
                                const Player& player,
                                std::span<const Sprite> sprites,
                                std::span<const double> depthBuffer,
                                const TextureLookup& textures,
                                const FogSettings& fog = {});
    };

}
