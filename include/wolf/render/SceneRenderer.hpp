#pragma once

#include "wolf/effects/Fog.hpp"
#include "wolf/render/TextureLookup.hpp"
#include "wolf/sprite/Sprite.hpp"

#include <span>

namespace wolf {

class FrameBuffer;
class Raycaster;
struct Player;

class SceneRenderer {
public:
    // One CPU-only drawing pass for lab 2. The caller owns all scene objects,
    // performs Raycaster::castFrame first, and presents the resulting framebuffer.
    static void drawFrame(FrameBuffer& frame, const Raycaster& raycaster,
                          const Player& player, std::span<const Sprite> sprites,
                          const TextureLookup& textures,
                          const FogSettings& fog = {});
};

} // namespace wolf
