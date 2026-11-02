#pragma once

#include <vector>
#include "sprite.h"
#include "player.h"
#include "framebuffer.h"
#include "texturestore.h"

class SpriteRenderer {
public:
    void draw(
        const Sprite& sprite,
        Player& player,
        FrameBuffer& frameBuffer,
        const std::vector<double>& depthBuffer,
        double projectionDistance,
        const Texture& texture
    );
};