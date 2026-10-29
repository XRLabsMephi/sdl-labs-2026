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
    // Draw billboard sprites back-to-front into an existing wall frame.
    // depthBuffer contains the nearest wall's perpendicular depth for each column.
    static void drawSprites(FrameBuffer& frame, const Player& player,
                            std::span<const Sprite> sprites,
                            std::span<const double> depthBuffer,
                            const TextureLookup& textures,
                            const FogSettings& fog = {});
};

} // namespace wolf
