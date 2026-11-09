#pragma once

#include <span>

#include "wolf/render/framebuffer.hpp"
#include "wolf/sprite/Sprite.hpp"
#include "wolf/world/Player.hpp"

namespace texture { class TextureStore; }

namespace sprite {
    class SpriteRenderer {
    public:
        // wallDepth contains the nearest fully opaque wall per column, in camera depth.
        // Pixel depth additionally masks alpha-cutout walls; glass resolves in the final pass.
        void render(const world::Player& player,
                    std::span<const Sprite> sprites,
                    const texture::TextureStore& textures,
                    std::span<const double> wallDepth,
                    render::Framebuffer& framebuffer) const;
    };
}
