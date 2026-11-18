#pragma once
#include "wolf/sprite/Sprite.hpp"
#include "wolf/world/Player.hpp"
#include "wolf/render/FrameBuffer.hpp"
#include "wolf/texture/TextureStore.hpp"
#include <vector>
#include "wolf/effects/Fog.hpp"

namespace wolf::sprite {
    class SpriteRenderer {
    public:
        void render(render::FrameBuffer& fb, const world::Player& player, const std::vector<Sprite>& sprites,
            const std::vector<double>& depthBuffer, const texture::TextureStore& textureStore, const effects::Fog& fog);
    };
}